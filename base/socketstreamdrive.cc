/* $Header: /var/cvs/mbdyn/mbdyn/mbdyn-1.0/mbdyn/base/socketstreamdrive.cc,v 1.55 2014/12/03 20:49:38 masarati Exp $ */
/* 
 * MBDyn (C) is a multibody analysis code. 
 * http://www.mbdyn.org
 *
 * Copyright (C) 1996-2014
 *
 * Pierangelo Masarati	<masarati@aero.polimi.it>
 * Paolo Mantegazza	<mantegazza@aero.polimi.it>
 *
 * Dipartimento di Ingegneria Aerospaziale - Politecnico di Milano
 * via La Masa, 34 - 20156 Milano, Italy
 * http://www.aero.polimi.it
 *
 * Changing this copyright notice is forbidden.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation (version 2 of the License).
 * 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * Michele Attolico <attolico@aero.polimi.it>
 */

/*
 * (Portions)
 *
 * AUTHOR: Dr. Rudolf Jaeger <rudijaeger@yahoo.com>
 * Copyright (C) 2008 all rights reserved.
 *
 * The copyright of this patch is trasferred
 * to Pierangelo Masarati and Paolo Mantegazza
 * for use in the software MBDyn as described 
 * in the GNU Public License version 2.1
 */

#include "mbconfig.h"           /* This goes first in every *.c,*.cc file */

#ifdef USE_SOCKET

#include "dataman.h"
#include "filedrv.h"
#include "streamdrive.h"
#include "sock.h"
#include "socketstreamdrive.h"

#include <string.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/un.h>
#include <arpa/inet.h>

#include "rtai_in_drive.h"

#define DEFAULT_PORT	9012	/* intentionally unassigned by IANA */
#define DEFAULT_HOST	"127.0.0.1"

SocketStreamDrive::SocketStreamDrive(unsigned int uL,
	const DriveHandler* pDH,
	UseSocket *pUS, bool c,
	const std::string& sFileName,
	integer nd, const std::vector<doublereal>& v0,
	unsigned int ie, bool bReceiveFirst,
	int flags,
	const struct timeval& st,
	const std::string& sOutFileName, int iPrecision,
	doublereal dShift)
: StreamDrive(uL, pDH, sFileName, nd, v0, c),
InputEvery(ie), bReceiveFirst(bReceiveFirst), InputCounter(ie - 1),
pUS(pUS), recv_flags(flags),
SocketTimeout(st),
sOutFileName(sOutFileName), iPrecision(iPrecision), dShift(dShift)
{
	// NOTE: InputCounter is set to InputEvery - 1 so that input
	// is expected at initialization (initial time) and then every
	// InputEvery steps; for example, for InputEvery == 4, input
	// is expected at:
	//	initial time
	//	initial time + 4 * timestep
	//	initial time + 8 * timestep
	ASSERT(InputEvery > 0);

	if (!bReceiveFirst) {
		InputCounter -= InputEvery;
	}

	if (!sOutFileName.empty()) {
		outFile.open(sOutFileName.c_str());
		if (!outFile) {
			silent_cerr("SocketStreamDrive(" << uLabel << "): "
				"unable to open echo file '" << sOutFileName << "'" << std::endl);
			throw ErrGeneric(MBDYN_EXCEPT_ARGS);
		}

		if (iPrecision > 0) {
			outFile.precision(iPrecision);
		}
		outFile.setf(std::ios::scientific);

		outFile
			<< "# generated by SocketStreamDrive(" << uLabel << ")"
			<< std::endl;
		if (nd == 1) {
			outFile
				<< "# Time, Channel #1"
				<< std::endl;

		} else {
			outFile
				<< "# Time, Channels #1-" << nd
				<< std::endl;
		}
	}
}

SocketStreamDrive::~SocketStreamDrive(void)
{
	if (pUS != 0) {
		SAFEDELETE(pUS);
	}
}

/* Scrive il contributo del DriveCaller al file di restart */   
std::ostream&
SocketStreamDrive::Restart(std::ostream& out) const
{
	out << "  file: " << uLabel << ", socket stream," 
		" stream drive name, \"" << sFileName << "\"";
	pUS->Restart(out);
	return out << ", " << iNumDrives << ";" << std::endl;
}
   
void
SocketStreamDrive::ServePending(const doublereal& t)
{
	
	// by now, an abandoned drive is not read any more;
	// should we retry or what?
	if (pUS->Abandoned()) {
		silent_cout("SocketStreamDrive(" << sFileName << "): "
			"abandoned"  << std::endl);
		return;
	}

	ASSERT(pUS->Connected());
	
	/* read only every InputEvery steps */
	InputCounter++;
	if (InputCounter != InputEvery) {
		return;
	}
	InputCounter = 0;
	
	int sock_nr = pUS->GetSock();
	int rc = -1;
	// Use socket timeout if set in input file; Default: 0
	if (SocketTimeout.tv_sec || SocketTimeout.tv_usec) {
		// Use Select() on the socket for automatic shutdown if
		// socket clients fail.
		fd_set readfds;

		// Clear the set
		FD_ZERO(&readfds);

		// Add descriptors to the set
		FD_SET(sock_nr, &readfds);

		// Copy timeout because select(2) may overwrite it
		struct timeval tv = SocketTimeout;

		// Call select
		rc = select(sock_nr + 1, &readfds, NULL, NULL, &tv);
		switch (rc) {
		case -1: {
			int save_errno = errno;
			char *err_msg = strerror(save_errno);

			silent_cout("SocketStreamDrive"
				"(" << sFileName << "): select failed"
				<< " (" << save_errno << ": " 
				<< err_msg << ")" << std::endl);
			throw ErrGeneric(MBDYN_EXCEPT_ARGS);
			}

		case 0:
			silent_cout("SocketStreamDrive"
				"(" << sFileName << "): select timed out"
				<< std::endl);
			throw ErrGeneric(MBDYN_EXCEPT_ARGS);

		default:
			if (!FD_ISSET(sock_nr, &readfds)) {
				silent_cout("SocketStreamDrive"
					"(" << sFileName << "): "
					"socket " << sock_nr << " reset"
					<< std::endl);
				throw ErrGeneric(MBDYN_EXCEPT_ARGS);
			}
		}
	}

	// Read data
	// NOTE: flags __SHOULD__ contain MSG_WAITALL;
	// however, it is not defined on some platforms (e.g. Cygwin)
	// TODO: needs work for network independence!
	rc = recv(sock_nr, buf, size, recv_flags);

	/* FIXME: no receive at first step? */
	switch (rc) {
	case 0:
do_abandon:;
		silent_cout("SocketStreamDrive(" << sFileName << "): "
			<< "communication closed by host; abandoning..."
			<< std::endl);
		pUS->Abandon();
		break;

	case -1: {
		int save_errno = errno;

		// some errno values may be legal
		switch (save_errno) {
		case EAGAIN:
			if (recv_flags & MSG_DONTWAIT) {
				// non-blocking
				return;
			}
			break;

		case ECONNRESET:
			goto do_abandon;
		}

		char *err_msg = strerror(save_errno);
		silent_cout("SocketStreamDrive(" << sFileName << ") failed "
				"(" << save_errno << ": " << err_msg << ")"
				<< std::endl);
		throw ErrGeneric(MBDYN_EXCEPT_ARGS);
		}

	default: {
		doublereal *rbuf = (doublereal *)buf - 1;

		// check whether echo is needed
		if (!sOutFileName.empty()) {
			for (int i = 1; i <= iNumDrives; i++) {
				if (pdVal[i] != rbuf[i]) {
					// changed; need to write
					outFile << (pDrvHdl->dGetTime() + dShift);
					for (int i = 1; i <= iNumDrives; i++) {
						outFile << " " << rbuf[i];
					}
					outFile << std::endl;
					break;
				}
			}
		}

		// copy values from buffer
		for (int i = 1; i <= iNumDrives; i++) {
			pdVal[i] = rbuf[i];
		}

		} break;
	}
}


/* legge i drivers tipo stream */

static Drive *
ReadStreamDrive(const DataManager *pDM, MBDynParser& HP, unsigned uLabel)
{
	bool create = false;
	unsigned short int port = -1; 
	std::string name;
	std::string host;
	std::string path;

	if (HP.IsKeyWord("name") || HP.IsKeyWord("stream" "drive" "name")) {
		const char *m = HP.GetStringWithDelims();
		if (m == 0) {
			silent_cerr("SocketStreamDrive(" << uLabel << "): "
				"unable to read stream drive name "
				"at line " << HP.GetLineData()
				<< std::endl);
			throw ErrGeneric(MBDYN_EXCEPT_ARGS);

		} 

		name = m;

	} else {
		silent_cerr("SocketStreamDrive(" << uLabel << "):"
			"missing stream drive name "
			"at line " << HP.GetLineData()
			<< std::endl);
		throw ErrGeneric(MBDYN_EXCEPT_ARGS);
	}

	if (HP.IsKeyWord("create")) {
		if (!HP.GetYesNo(create)) {
			silent_cerr("SocketStreamDrive"
				"(" << uLabel << ", \"" << name << "\"): "
				"\"create\" must be either \"yes\" or \"no\" "
				"at line " << HP.GetLineData()
				<< std::endl);
			throw ErrGeneric(MBDYN_EXCEPT_ARGS);
		}
	}
		
	if (HP.IsKeyWord("local") || HP.IsKeyWord("path")) {
		const char *m = HP.GetFileName();
		
		if (m == 0) {
			silent_cerr("SocketStreamDrive"
				"(" << uLabel << ", \"" << name << "\"): "
				"unable to read local path"
				"at line " << HP.GetLineData()
				<< std::endl);
			throw ErrGeneric(MBDYN_EXCEPT_ARGS);
		}
		
		path = m;
	}
	
	if (HP.IsKeyWord("port")) {
		if (!path.empty()) {
			silent_cerr("SocketStreamDrive"
				"(" << uLabel << ", \"" << name << "\"): "
				"cannot specify port "
				"for a local socket "
				"at line " << HP.GetLineData()
				<< std::endl);
			throw ErrGeneric(MBDYN_EXCEPT_ARGS);		
		}

		int p = HP.GetInt();
		/* Da sistemare da qui */
#ifdef IPPORT_USERRESERVED
		if (p <= IPPORT_USERRESERVED) {
			silent_cerr("SocketStreamDrive"
				"(" << uLabel << ", \"" << name << "\"): "
				"cannot listen on reserved port "
				<< port << ": less than "
				"IPPORT_USERRESERVED=" << IPPORT_USERRESERVED
				<< " at line " << HP.GetLineData()
				<< std::endl);
			throw ErrGeneric(MBDYN_EXCEPT_ARGS);
		}
		/* if #undef'd, don't bother checking;
		 * the OS will do it for us */
#endif /* IPPORT_USERRESERVED */

		port = p;
	}

	
	if (HP.IsKeyWord("host")) {
		if (!path.empty()) {
			silent_cerr("SocketStreamDrive"
				"(" << uLabel << ", \"" << name << "\"): "
				"cannot specify host for a local socket "
				"at line " << HP.GetLineData()
				<< std::endl);
			throw ErrGeneric(MBDYN_EXCEPT_ARGS);		
		}

		const char *h;
		
		h = HP.GetStringWithDelims();
		if (h == 0) {
			silent_cerr("SocketStreamDrive"
				"(" << uLabel << ", \"" << name << "\"): "
				"unable to read host "
				"at line " << HP.GetLineData()
				<< std::endl);
			throw ErrGeneric(MBDYN_EXCEPT_ARGS);
		}

		host = h;

	} else if (path.empty() && !create) {
		silent_cerr("SocketStreamDrive"
			"(" << uLabel << ", \"" << name << "\"): "
			"host undefined, "
			"using default \"" << DEFAULT_HOST "\" "
			"at line " << HP.GetLineData()
			<< std::endl);
		host = DEFAULT_HOST;
	}

	// we want to block until the whole chunk is received
	int flags = 0;
#ifdef MSG_WAITALL
	flags |= MSG_WAITALL;
#endif // MSG_WAITALL

	while (HP.IsArg()) {
		if (HP.IsKeyWord("signal")) {
#ifdef MSG_NOSIGNAL
			flags &= ~MSG_NOSIGNAL;
#else // ! MSG_NOSIGNAL
			silent_cout("SocketStreamDrive"
				"(" << uLabel << ", \"" << name << "\"): "
				"MSG_NOSIGNAL not defined (ignored) "
				"at line " << HP.GetLineData()
				<< std::endl);
#endif // ! MSG_NOSIGNAL

		// not honored by recv(2)
		} else if (HP.IsKeyWord("no" "signal")) {
#ifdef MSG_NOSIGNAL
			flags |= MSG_NOSIGNAL;
#else // ! MSG_NOSIGNAL
			silent_cout("SocketStreamDrive"
				"(" << uLabel << ", \"" << name << "\"): "
				"MSG_NOSIGNAL not defined (ignored) "
				"at line " << HP.GetLineData()
				<< std::endl);
#endif // ! MSG_NOSIGNAL

		} else if (HP.IsKeyWord("blocking")) {
			// not honored by recv(2)?
			flags |= MSG_WAITALL;
			flags &= ~MSG_DONTWAIT;

		} else if (HP.IsKeyWord("non" "blocking")) {
			// not honored by recv(2)?
			flags &= ~MSG_WAITALL;
			flags |= MSG_DONTWAIT;

		} else {
			break;
		}
	}

	unsigned int InputEvery = 1;
	if (HP.IsKeyWord("input" "every")) {
		int i = HP.GetInt();
		if (i <= 0) {
			silent_cerr("SocketStreamDrive"
				"(" << uLabel << ", \"" << name << "\"): "
				"invalid \"input every\" value " << i
				<< " at line " << HP.GetLineData()
				<< std::endl);
			throw ErrGeneric(MBDYN_EXCEPT_ARGS);
		}
		InputEvery = (unsigned int)i;
	}

	bool bReceiveFirst(true);
	if (HP.IsKeyWord("receive" "first")) {
		if (!HP.GetYesNo(bReceiveFirst)) {
			silent_cerr("SocketStreamDrive"
				"(" << uLabel << ", \"" << name << "\"): "
				"\"receive first\" must be either \"yes\" or \"no\" "
				<< "at line " << HP.GetLineData()
				<< std::endl);
			throw ErrGeneric(MBDYN_EXCEPT_ARGS);
		}
	}

	struct timeval SocketTimeout = { 0, 0 };
	if (HP.IsKeyWord("timeout")) {
		doublereal st = HP.GetReal();
		if (st < 0) {
			silent_cerr("SocketStreamDrive"
				"(" << uLabel << ", \"" << name << "\"): "
				"invalid socket timeout value " << st
				<< " at line " << HP.GetLineData()
				<< std::endl);
			throw ErrGeneric(MBDYN_EXCEPT_ARGS);
		}
		SocketTimeout.tv_sec = long(st);
		SocketTimeout.tv_usec = long((st - SocketTimeout.tv_sec)*1000000);
	}

	pedantic_cout("SocketStreamDrive"
		"(" << uLabel << ", \"" << name << "\"): "
		"timeout: " << SocketTimeout.tv_sec << "s "
		<< SocketTimeout.tv_usec << "ns" << std::endl);

	std::string sOutFileName;
	int iPrecision = -1;
	doublereal dShift = 0.;
	if (HP.IsKeyWord("echo")) {
		const char *s = HP.GetFileName();
		if (s == NULL) {
			silent_cerr("SocketStreamDrive"
				"(" << uLabel << ", \"" << name << "\"): "
				"unable to parse echo file name "
				"at line " << HP.GetLineData()
				<< std::endl);
			throw ErrGeneric(MBDYN_EXCEPT_ARGS);
		}

		sOutFileName = s;

		if (HP.IsKeyWord("precision")) {
			iPrecision = HP.GetInt();
			if (iPrecision <= 0) {
				silent_cerr("SocketStreamDrive"
					"(" << uLabel << ", \"" << name << "\"): "
					"invalid echo precision " << iPrecision
					<< " at line " << HP.GetLineData()
					<< std::endl);
				throw ErrGeneric(MBDYN_EXCEPT_ARGS);
			}
		}

		if (HP.IsKeyWord("shift")) {
			dShift = HP.GetReal();
		}
	}

	int idrives = HP.GetInt();
	if (idrives <= 0) {
		silent_cerr("SocketStreamDrive"
			"(" << uLabel << ", \"" << name << "\"): "
			"illegal number of channels " << idrives
			<< " at line " << HP.GetLineData()
			<< std::endl);
		throw ErrGeneric(MBDYN_EXCEPT_ARGS);
	}

	std::vector<doublereal> v0;
	if (HP.IsKeyWord("initial" "values")) {
		v0.resize(idrives);
		for (int i = 0; i < idrives; i++) {
			v0[i] = HP.GetReal();
		}
	}

	UseSocket *pUS = 0;
	if (path.empty()) {
		if (port == (unsigned short int)(-1)) {
			port = DEFAULT_PORT;
		}
		SAFENEWWITHCONSTRUCTOR(pUS, UseInetSocket, UseInetSocket(host, port, create));

	} else {
		SAFENEWWITHCONSTRUCTOR(pUS, UseLocalSocket, UseLocalSocket(path, create));
	}

	if (create) {
		const_cast<DataManager *>(pDM)->RegisterSocketUser(pUS);

	} else {
		pUS->Connect();
	}

	Drive* pDr = 0;
	SAFENEWWITHCONSTRUCTOR(pDr, SocketStreamDrive,
		SocketStreamDrive(uLabel,
			pDM->pGetDrvHdl(), pUS, create,
			name, idrives, v0, InputEvery, bReceiveFirst,
			flags, SocketTimeout,
			sOutFileName, iPrecision, dShift));

	return pDr;
}

Drive *
StreamDR::Read(unsigned uLabel, const DataManager *pDM, MBDynParser& HP)
{
	Drive *pDr = 0;

	if (!s.empty()) {
		pedantic_cout("\"" << s << "\" is deprecated; "
			"use \"stream\" instead at line "
			<< HP.GetLineData() << std::endl);
	}

#ifdef USE_RTAI
	if (::rtmbdyn_rtai_task != NULL){
		silent_cout("starting RTMBDyn drive " << uLabel << std::endl);
		pDr = ReadRTMBDynInDrive(pDM, HP, uLabel);
	} else 
#endif /* USE_RTAI */		
	{
#ifdef USE_SOCKET
		silent_cout("starting stream drive " << uLabel << std::endl);
		pDr = ReadStreamDrive(pDM, HP, uLabel);
#else // ! USE_SOCKET
		silent_cerr("stream drive " << uLabel
			<< " not allowed at line " << HP.GetLineData()
			<< " because apparently the current architecture "
			"does not support sockets" << std::endl);
		throw ErrGeneric(MBDYN_EXCEPT_ARGS);
#endif // ! USE_SOCKET
	}

	return pDr;
}

#endif // USE_SOCKET
