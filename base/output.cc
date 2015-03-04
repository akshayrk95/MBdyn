/* $Header: /var/cvs/mbdyn/mbdyn/mbdyn-1.0/mbdyn/base/output.cc,v 1.60 2014/07/22 19:19:18 masarati Exp $ */
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

/* gestore dell'output */

#include "mbconfig.h"           /* This goes first in every *.c,*.cc file */

#include "output.h"

/* OutputHandler - begin */

#ifndef OUTPUT_PRECISION
#define OUTPUT_PRECISION 6
#endif /* OUTPUT_PRECISION */

const int iDefaultWidth = OUTPUT_PRECISION + 6;
const int iDefaultPrecision = OUTPUT_PRECISION;

const char* psExt[] = {
	".out",		//  0
	".mov",
	".ele",
	".abs",
	".ine",
	".jnt",		//  5
	".frc",
	".act",
	".rot",
	".rst",
	".rst.X",	// 10
	".aer",
	".hyd",
	".prs",
	".usr",
	".gen",		// 15
	".par",
	".res",
	".ada",
	".amd",
	".rfm",		// 20
	".log",
	".air",
	".prm",
	".ext",
	".mod",		// 25
	".nc",
	".thn",
	".the",
	".pla",
	".grv",		// 30
	".dof",
	".drv",
	".trc",
	NULL		// 34
};

/* Costruttore senza inizializzazione */
OutputHandler::OutputHandler(void)
: FileName(NULL),
#ifdef USE_NETCDF
m_DimTime(0),
m_DimV1(0),
m_DimV3(0),
m_pBinFile(0),
#endif /* USE_NETCDF */
iCurrWidth(iDefaultWidth),
iCurrPrecision(iDefaultPrecision),
nCurrRestartFile(0)
{
	OutputHandler_int();
}

/* Costruttore con inizializzazione */
OutputHandler::OutputHandler(const char* sFName, int iExtNum)
: FileName(sFName, iExtNum),
#ifdef USE_NETCDF
m_DimTime(0),
m_DimV1(0),
m_DimV3(0),
m_pBinFile(0),
#endif /* USE_NETCDF */
iCurrWidth(iDefaultWidth),
iCurrPrecision(iDefaultPrecision),
nCurrRestartFile(0)
{
	OutputHandler_int();
	Init(sFName, iExtNum);
}

// Pesudo-constructor
void
OutputHandler::OutputHandler_int(void)
{
	OutData[OUTPUT].flags = OUTPUT_USE_DEFAULT_PRECISION
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT;
	OutData[OUTPUT].pof = &ofOutput;

	OutData[STRNODES].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT
		| OUTPUT_MAY_USE_NETCDF;
	OutData[STRNODES].pof = &ofStrNodes;

	OutData[ELECTRIC].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT;
	OutData[ELECTRIC].pof= &ofElectric;

	OutData[THERMALNODES].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT;
	OutData[THERMALNODES].pof= &ofThermalNodes;

	OutData[THERMALELEMENTS].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT;
	OutData[THERMALELEMENTS].pof= &ofThermalElements;

	OutData[ABSTRACT].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT;
	OutData[ABSTRACT].pof = &ofAbstract;

	OutData[INERTIA].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT
		| OUTPUT_MAY_USE_NETCDF;
	OutData[INERTIA].pof = &ofInertia;

	OutData[JOINTS].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT
		| OUTPUT_MAY_USE_NETCDF;
	OutData[JOINTS].pof = &ofJoints;

	OutData[FORCES].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT
		| OUTPUT_MAY_USE_NETCDF;
	OutData[FORCES].pof = &ofForces;

	OutData[BEAMS].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT
		| OUTPUT_MAY_USE_NETCDF;
	OutData[BEAMS].pof = &ofBeams;

	OutData[ROTORS].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT;
	OutData[ROTORS].pof = &ofRotors;

	OutData[RESTART].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT;
	OutData[RESTART].pof = &ofRestart;

	OutData[RESTARTXSOL].flags = 0
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT;
	OutData[RESTARTXSOL].pof = &ofRestartXSol;

	OutData[AERODYNAMIC].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT
		| OUTPUT_MAY_USE_NETCDF;
	OutData[AERODYNAMIC].pof = &ofAerodynamic;

	OutData[HYDRAULIC].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT;
	OutData[HYDRAULIC].pof = &ofHydraulic;

	OutData[PRESNODES].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT;
	OutData[PRESNODES].pof = &ofPresNodes;

	OutData[LOADABLE].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT
		| OUTPUT_MAY_USE_NETCDF;
	OutData[LOADABLE].pof = &ofLoadable;

	OutData[GENELS].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT;
	OutData[GENELS].pof = &ofGenels;

	OutData[PARTITION].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT;
	OutData[PARTITION].pof = &ofPartition;

	OutData[ADAMSRES].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT;
	OutData[ADAMSRES].pof = &ofAdamsRes;

	OutData[ADAMSCMD].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT;
	OutData[ADAMSCMD].pof = &ofAdamsCmd;

	OutData[AEROMODALS].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT;
	OutData[AEROMODALS].pof = &ofAeroModals;

	OutData[REFERENCEFRAMES].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT;
	OutData[REFERENCEFRAMES].pof = &ofReferenceFrames;

	OutData[LOG].flags = 0
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT;
	OutData[LOG].pof = &ofLog;

	OutData[AIRPROPS].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT;
	OutData[AIRPROPS].pof = &ofAirProps;

	OutData[PARAMETERS].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT;
	OutData[PARAMETERS].pof = &ofParameters;

	OutData[EXTERNALS].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT;
	OutData[EXTERNALS].pof = &ofExternals;

	OutData[MODAL].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT;
	OutData[MODAL].pof = &ofModal;

	OutData[PLATES].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT
		| OUTPUT_MAY_USE_NETCDF;
	OutData[PLATES].pof = &ofPlates;

	OutData[GRAVITY].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
		| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT;
	OutData[GRAVITY].pof = &ofGravity;

	OutData[DOFSTATS].flags = OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT;
	OutData[DOFSTATS].pof = &ofDofStats;

	OutData[DRIVECALLERS].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
			| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT;
	OutData[DRIVECALLERS].pof = &ofDriveCallers;

	OutData[TRACES].flags = OUTPUT_USE_DEFAULT_PRECISION | OUTPUT_USE_SCIENTIFIC
			| OUTPUT_MAY_USE_TEXT | OUTPUT_USE_TEXT;
	OutData[TRACES].pof = &ofTraces;

	OutData[NETCDF].flags = 0
		| OUTPUT_MAY_USE_NETCDF;
	OutData[NETCDF].pof = 0;

	currentStep = 0;
}

/* Inizializzazione */
void
OutputHandler::Init(const char* sFName, int iExtNum)
{
	FileName::iInit(sFName, iExtNum);

	OutputOpen();
	LogOpen();
}

/* Distruttore */
OutputHandler::~OutputHandler(void)
{
	for (int iCnt = 0; iCnt < LASTFILE; iCnt++) {
		if (IsOpen(iCnt)) {
#ifdef USE_NETCDF
			if (iCnt == NETCDF) {
				delete m_pBinFile;

			} else
#endif /* USE_NETCDF */
			{
				OutData[iCnt].pof->close();
			}
		}
	}
}

/* Aggiungere qui le funzioni che aprono i singoli stream */
bool
OutputHandler::Open(const OutputHandler::OutFiles out)
{
	if (!IsOpen(out)) {
#ifdef USE_NETCDF
		if (out == NETCDF) {
			m_pBinFile = new NcFile(_sPutExt((char*)(psExt[NETCDF])), NcFile::Replace);
			m_pBinFile->set_fill(NcFile::Fill);
	
         		if (!m_pBinFile->is_valid()) {
				silent_cerr("NetCDF file is invalid" << std::endl);
				throw ErrFile(MBDYN_EXCEPT_ARGS);
			}

			// Let's define some dimensions which could be useful	       	     
			m_DimTime = CreateDim("time");
			m_DimV1 = CreateDim("vec1", 1);
			m_DimV3 = CreateDim("vec3", 3);

		} else
#endif /* USE_NETCDF */
		{
			const char *fname = _sPutExt(psExt[out]);

			// Apre lo stream
			OutData[out].pof->open(fname);

			if (!(*OutData[out].pof)) {
				silent_cerr("Unable to open file "
					"\"" << fname << "\"" << std::endl);
				throw ErrFile(MBDYN_EXCEPT_ARGS);
			}
		}

		if (UseText(out)) {
			// Setta la formattazione dei campi
			if (UseDefaultPrecision(out)) {
				OutData[out].pof->precision(iCurrPrecision);
			}

			// Setta la notazione
			if (UseScientific(out)) {
				OutData[out].pof->setf(std::ios::scientific);
			}
		}

		return true;
	}

	return false;
}

bool
OutputHandler::IsOpen(int out) const
{
	ASSERT(out > OutputHandler::UNKNOWN);
	ASSERT(out < OutputHandler::LASTFILE);

	return IsOpen(OutputHandler::OutFiles(out));
}

bool
OutputHandler::IsOpen(const OutputHandler::OutFiles out) const
{
#ifdef USE_NETCDF
	if (out == NETCDF) {
		return m_pBinFile == 0 ? false : m_pBinFile->is_valid();
	}
#endif /* USE_NETCDF */

	return OutData[out].pof == 0 ? false : OutData[out].pof->is_open();
}

bool
OutputHandler::UseScientific(int out) const
{
	ASSERT(out > OutputHandler::UNKNOWN);
	ASSERT(out < OutputHandler::LASTFILE);

	return UseScientific(OutputHandler::OutFiles(out));
}

bool
OutputHandler::UseScientific(const OutputHandler::OutFiles out) const
{
	return (OutData[out].flags & OUTPUT_USE_SCIENTIFIC);
}

bool
OutputHandler::UseDefaultPrecision(int out) const
{
	ASSERT(out > OutputHandler::UNKNOWN);
	ASSERT(out < OutputHandler::LASTFILE);

	return UseDefaultPrecision(OutputHandler::OutFiles(out));
}

bool
OutputHandler::UseDefaultPrecision(const OutputHandler::OutFiles out) const
{
	return (OutData[out].flags & OUTPUT_USE_DEFAULT_PRECISION);
}

bool
OutputHandler::UseText(int out) const
{
	ASSERT(out > OutputHandler::UNKNOWN);
	ASSERT(out < OutputHandler::LASTFILE);

	return UseText(OutputHandler::OutFiles(out));
}

void
OutputHandler::SetText(const OutputHandler::OutFiles out)
{
	if (!(OutData[out].flags & OUTPUT_MAY_USE_TEXT)) {
		throw ErrGeneric(MBDYN_EXCEPT_ARGS);
	}

	OutData[out].flags |= OUTPUT_USE_TEXT;
}

void
OutputHandler::ClearText(void)
{
	for (int out = FIRSTFILE; out < LASTFILE; out++) {
		if (OutData[out].flags & OUTPUT_MAY_USE_TEXT) {
			OutData[out].flags &= ~OUTPUT_USE_TEXT;
		}
	}
}

void
OutputHandler::ClearText(const OutputHandler::OutFiles out)
{
	if (!(OutData[out].flags & OUTPUT_MAY_USE_TEXT)) {
		throw ErrGeneric(MBDYN_EXCEPT_ARGS);
	}

	OutData[out].flags &= ~OUTPUT_USE_TEXT;
}

bool
OutputHandler::UseText(const OutputHandler::OutFiles out) const
{
	return (OutData[out].flags & OUTPUT_USE_TEXT);
}

bool
OutputHandler::UseNetCDF(int out) const
{
	ASSERT(out > OutputHandler::UNKNOWN);
	ASSERT(out < OutputHandler::LASTFILE);

	return UseNetCDF(OutputHandler::OutFiles(out));
}

void
OutputHandler::SetNetCDF(const OutputHandler::OutFiles out)
{
	if (!(OutData[out].flags & OUTPUT_MAY_USE_NETCDF)) {
		throw ErrGeneric(MBDYN_EXCEPT_ARGS);
	}

	OutData[out].flags |= OUTPUT_USE_NETCDF;
}

void
OutputHandler::ClearNetCDF(void)
{
	for (int out = FIRSTFILE; out < LASTFILE; out++) {
		if (OutData[out].flags & OUTPUT_MAY_USE_NETCDF) {
			OutData[out].flags &= ~OUTPUT_USE_NETCDF;
		}
	}
}

void
OutputHandler::ClearNetCDF(const OutputHandler::OutFiles out)
{
	if (!(OutData[out].flags & OUTPUT_MAY_USE_NETCDF)) {
		throw ErrGeneric(MBDYN_EXCEPT_ARGS);
	}

	OutData[out].flags &= ~OUTPUT_USE_NETCDF;
}

bool
OutputHandler::UseNetCDF(const OutputHandler::OutFiles out) const
{
	return (OutData[out].flags & OUTPUT_USE_NETCDF);
}

bool
OutputHandler::Close(const OutputHandler::OutFiles out)
{
	if (!IsOpen(out)) {
		return false;
	}

#ifdef USE_NETCDF
	if (out == NETCDF) {
		m_pBinFile->close();

	} else
#endif /* USE_NETCDF */
	{
		// Chiude lo stream
		OutData[out].pof->close();
	}

	return true;
}

bool
OutputHandler::OutputOpen(void)
{
	return Open(OUTPUT);
}

bool
OutputHandler::RestartOpen(bool openResXSol)
{
	if (!IsOpen(RESTART)) {
		char *resExt = NULL;
		int n = nCurrRestartFile > 0 ?
			(int)log10(nCurrRestartFile) + 1 : 1;
		int lenExt = STRLENOF(".")
			+ n
			+ STRLENOF(".rst")
			+ 1;

		SAFENEWARR(resExt, char, lenExt);
		snprintf(resExt, lenExt, ".%.*d.rst", n, nCurrRestartFile);
		/* Apre lo stream */
	      	OutData[RESTART].pof->open(_sPutExt(resExt));

	      	if(!(*OutData[RESTART].pof)) {
		 	std::cerr << "Unable to open file '" << _sPutExt(resExt)
		   		<< '\'' << std::endl;
			throw ErrFile(MBDYN_EXCEPT_ARGS);
		}
		SAFEDELETEARR(resExt);

		/* Setta la formattazione dei campi */
		if(UseDefaultPrecision(RESTART)) {
			OutData[RESTART].pof->precision(iCurrPrecision);
		}

		/* Setta la notazione */
		if(UseScientific(RESTART)) {
			OutData[RESTART].pof->setf(std::ios::scientific);
		}

		if (openResXSol) {
			ASSERT(!IsOpen(RESTARTXSOL));

			char *resXSolExt = NULL;
			int n = nCurrRestartFile > 0 ?
				(int)log10(nCurrRestartFile) + 1 : 1;
			int lenXSolExt = STRLENOF(".")
				+ n
				+ STRLENOF(".rst.X")
				+ 1;

			SAFENEWARR(resXSolExt, char, lenXSolExt);
			snprintf(resXSolExt, lenXSolExt, ".%.*d.rst.X", n, nCurrRestartFile);
			/* Apre lo stream */
		      	OutData[RESTARTXSOL].pof->open(_sPutExt(resXSolExt));
		      	if(!(*OutData[RESTARTXSOL].pof)) {
			 	std::cerr << "Unable to open file '" << _sPutExt(resExt)
			   		<< '\'' << std::endl;
				throw ErrFile(MBDYN_EXCEPT_ARGS);
			}
			SAFEDELETEARR(resXSolExt);
			/* non occorre settare la precisione e il formato
			perche' il file e' binario */
		}

		nCurrRestartFile++;

 	     	return false;
	}

	return true;
}

bool
OutputHandler::PartitionOpen(void)
{
	ASSERT(!IsOpen(PARTITION));
	return Open(PARTITION);
}

bool
OutputHandler::AdamsResOpen(void)
{
	ASSERT(!IsOpen(ADAMSRES));
	return Open(ADAMSRES);
}

bool
OutputHandler::AdamsCmdOpen(void)
{
	ASSERT(!IsOpen(ADAMSCMD));
	return Open(ADAMSCMD);
}

bool
OutputHandler::LogOpen(void)
{
	ASSERT(!IsOpen(LOG));
	return Open(LOG);
}

/* Setta precisione e dimensioni campo */
const int iWidth = 7; /* Caratteri richiesti dalla notazione esponenziale */

void
OutputHandler::SetWidth(int iNewWidth)
{
	ASSERT(iNewWidth > iWidth);
	if (iNewWidth > iWidth) {
		iCurrWidth = iNewWidth;
		iCurrPrecision = iCurrWidth-iWidth;
		for (int iCnt = 0; iCnt < LASTFILE; iCnt++) {
			if (UseDefaultPrecision(iCnt) && IsOpen(iCnt)) {
				OutData[iCnt].pof->width(iCurrWidth);
				OutData[iCnt].pof->precision(iCurrPrecision);
			}
		}
	}
}

void
OutputHandler::SetPrecision(int iNewPrecision)
{
	ASSERT(iNewPrecision > 0);
	if (iNewPrecision > 0) {
		iCurrPrecision = iNewPrecision;
		iCurrWidth = iNewPrecision + iWidth;
		for (int iCnt = 0; iCnt < LASTFILE; iCnt++) {
			if (UseDefaultPrecision(iCnt) && IsOpen(iCnt)) {
				OutData[iCnt].pof->width(iCurrWidth);
				OutData[iCnt].pof->precision(iCurrPrecision);
			}
		}
	}
}

#ifdef USE_NETCDF
const NcDim *
OutputHandler::CreateDim(const std::string& name, integer size)
{
	ASSERT(m_pBinFile != 0);

	NcDim *dim;
	if (size == -1) {
		dim = m_pBinFile->add_dim(name.c_str());

	} else {
		dim = m_pBinFile->add_dim(name.c_str(), size);
	}

	if (dim == 0) {
		std::ostringstream os;
		os << "OutputHandler::CreateDim(\"" << name << "\"";
		if (size > -1) {
			os << ", " << size;
		}
		os << "): unable to add dimension";
		silent_cerr(os.str() << std::endl);
		throw ErrGeneric(MBDYN_EXCEPT_ARGS);
	}

	return dim;
}

const NcDim *
OutputHandler::GetDim(const std::string& name) const
{
	ASSERT(m_pBinFile != 0);

	return m_pBinFile->get_dim(name.c_str());
}

NcVar *
OutputHandler::CreateVar(const std::string& name, NcType type,
	const AttrValVec& attrs, const NcDimVec& dims)
{
	NcVar *var;

	var = m_pBinFile->add_var(name.c_str(), type, dims.size(), const_cast<const NcDim **>(&dims[0]));
	if (var == 0) {
		silent_cerr("OutputHandler::CreateVar(\"" << name << "\") failed" << std::endl);
		throw ErrGeneric(MBDYN_EXCEPT_ARGS);
	}

	for (AttrValVec::const_iterator i = attrs.begin(); i != attrs.end(); i++) {
		if (!var->add_att(i->attr.c_str(), i->val.c_str())) {
			silent_cerr("OutputHandler::CreateVar(\"" << name << "\"): "
				"add_att(\"" << i->attr << "\", \"" << i->val << "\") failed" << std::endl);
			throw ErrGeneric(MBDYN_EXCEPT_ARGS);
		}
	}

	return var;
}

NcVar *
OutputHandler::CreateVar(const std::string& name, const std::string& type)
{
	AttrValVec attrs(1);
	attrs[0] = AttrVal("type", type);

	NcDimVec dims(1);
	dims[0] = DimV1();

	return CreateVar(name, ncChar, attrs, dims);
}

NcVar *
OutputHandler::CreateRotationVar(const std::string& name_prefix,
	const std::string& name_postfix,
	OrientationDescription od,
	const std::string& description)
{
	NcDimVec dim;
	AttrValVec attrs;
	std::string name(name_prefix);

	switch (od) {
	case ORIENTATION_MATRIX:
		dim.resize(3);
		dim[0] = DimTime();
		dim[1] = DimV3();
		dim[2] = DimV3();

		attrs.resize(3);
		attrs[0] = AttrVal("units", "-");
		attrs[1] = AttrVal("type", "Mat3x3");
		attrs[2] = AttrVal("description",
			description + " orientation matrix "
			"(R11, R21, R31, R12, R22, R32, R13, R23, R33)");

		name += "R";
		break;

	case ORIENTATION_VECTOR:
		dim.resize(2);
		dim[0] = DimTime();
		dim[1] = DimV3();

		attrs.resize(3);
		attrs[0] = AttrVal("units", "radian");
		attrs[1] = AttrVal("type", "Vec3");
		attrs[2] = AttrVal("description",
			description + " orientation vector "
			"(Phi_X, Phi_Y, Phi_Z)");

		name += "Phi";
		break;

	case EULER_123:
	case EULER_313:
	case EULER_321:
		{
		dim.resize(2);
		dim[0] = DimTime();
		dim[1] = DimV3();

		attrs.resize(3);
		attrs[0] = AttrVal("units", "radian");
		attrs[1] = AttrVal("type", "Vec3");

		std::string etype;
		switch (od) {
		case EULER_123:
			etype = "123";
			break;

		case EULER_313:
			etype = "313";
			break;

		case EULER_321:
			etype = "321";
			break;

		default:
			throw ErrGeneric(MBDYN_EXCEPT_ARGS);
		}

		attrs[2] = AttrVal("description",
			description + " orientation Euler angles (" + etype + ") "
			"(E_X, E_Y, E_Z)");

		name += "E";
		} break;

	default:
		throw ErrGeneric(MBDYN_EXCEPT_ARGS);
	}

	name += name_postfix;
	return CreateVar(name, ncDouble, attrs, dim);
}
#endif // USE_NETCDF

/* OutputHandler - end */


/* ToBeOutput - begin */

ToBeOutput::ToBeOutput(flag fOut)
: fOutput(fOut)
{
	NO_OP;
}

ToBeOutput::~ToBeOutput(void)
{
	NO_OP;
}

void
ToBeOutput::OutputPrepare(OutputHandler &OH)
{
	NO_OP;
}

/* Regular output */
void
ToBeOutput::Output(OutputHandler& OH) const
{
	NO_OP;
}

/* Output of perturbed solution (modes ...) */
void
ToBeOutput::Output(OutputHandler& OH,
		const VectorHandler& X, const VectorHandler& XP) const
{
	NO_OP;
}

flag
ToBeOutput::fToBeOutput(void) const
{
  	return fOutput;
}

void
ToBeOutput::SetOutputFlag(flag f)
{
  	fOutput = f;
}

Traceable::Traceable(flag fTrace)
:fTrace(fTrace)
{

}

Traceable::~Traceable(void)
{

}

flag Traceable::fToBeTraced(void) const
{
	return fTrace;
}

void Traceable::SetTraceFlag(flag f)
{
	fTrace = f;
}

/* ToBeOutput - end */
