/* $Header: /var/cvs/mbdyn/mbdyn/mbdyn-1.0/mbdyn/base/output.h,v 1.58 2014/07/22 19:19:18 masarati Exp $ */
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
 * ACKNOWLEDGEMENTS:
 * Support for output with NetCDF is based on a contribution
 * by Patrick Rix <patrick.rix@online.de>
 */

/* gestore dell'output */

#ifndef OUTPUT_H
#define OUTPUT_H

/* se #define DEBUG_COUT l'output avviene su cout anziche' nei files */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <typeinfo>

#ifdef USE_NETCDF
#if defined(HAVE_NETCDFCPP_H)
#include <netcdfcpp.h>  
#elif defined(HAVE_NETCDF_H)
#include <netcdf.h>  
#endif
#endif /* USE_NETCDF */

#include "myassert.h"
#include "except.h"
#include "solman.h"
#include "filename.h"

/* OutputHandler - begin */

class OutputHandler : public FileName {
public:
	enum OutFiles {
		UNKNOWN			= -1,
		FIRSTFILE		= 0,
		OUTPUT			= 0,	//  0
		STRNODES,
		ELECTRIC,
		ABSTRACT,
		INERTIA,
		JOINTS,				//  5
		FORCES,
		BEAMS,
		ROTORS,
		RESTART,
		RESTARTXSOL,			// 10
		AERODYNAMIC,
		HYDRAULIC,
		PRESNODES,
		LOADABLE,
		GENELS,				// 15
		PARTITION,
		ADAMSRES,
		ADAMSCMD,
		AEROMODALS,
		REFERENCEFRAMES,		// 20
		LOG,
		AIRPROPS,
		PARAMETERS,
		EXTERNALS,
		MODAL,				// 25
		NETCDF,
		THERMALNODES,
		THERMALELEMENTS,
		PLATES,
		GRAVITY,			// 30
		DOFSTATS,
		DRIVECALLERS,
		TRACES,
		LASTFILE			// 34
	};

private:
	long currentStep;

public:
	inline void SetCurrentStep(long Step) {
		currentStep = Step;
	};
	inline void IncCurrentStep(void) {
		currentStep++;
	};
       	inline long GetCurrentStep(void) const {
		return currentStep;
	};

private:

	// flag values
	enum {
		OUTPUT_NONE			= 0x00U,
		OUTPUT_USE_DEFAULT_PRECISION	= 0x01U,
		OUTPUT_USE_SCIENTIFIC		= 0x02U,

		OUTPUT_MAY_USE_TEXT		= 0x10U,
		OUTPUT_USE_TEXT			= 0x20U,
		OUTPUT_MAY_USE_NETCDF		= 0x40U,
		OUTPUT_USE_NETCDF		= 0x80U,

		LAST
	};

	/* Aggiungere qui i files che si desidera avere a disposizione */
	struct {
		std::ofstream*	pof;
		unsigned	flags;
	} OutData[LASTFILE];
		
	// NetCDF dimensions and global attributes related to the binary file
#ifdef USE_NETCDF
	const NcDim *m_DimTime;
	const NcDim *m_DimV1;
	const NcDim *m_DimV3;
	NcFile *m_pBinFile;   /* ! one ! binary NetCDF data file */
#endif /* USE_NETCDF */

	/* handlers to streams */
	std::ofstream ofOutput;      		/*  0 */
	std::ofstream ofStrNodes;
	std::ofstream ofElectric;
	std::ofstream ofAbstract;
	std::ofstream ofInertia;
	std::ofstream ofJoints;      		/*  5 */
	std::ofstream ofForces;
	std::ofstream ofBeams;
	std::ofstream ofRotors;
	std::ofstream ofRestart;
	std::ofstream ofRestartXSol; 		/* 10 */
	std::ofstream ofAerodynamic;
	std::ofstream ofHydraulic;
	std::ofstream ofPresNodes;
	std::ofstream ofLoadable;
	std::ofstream ofGenels;			/* 15 */
	std::ofstream ofPartition;
	std::ofstream ofAdamsRes;
	std::ofstream ofAdamsCmd;
	std::ofstream ofAeroModals;
	std::ofstream ofReferenceFrames;	/* 20 */
	std::ofstream ofLog;
	std::ofstream ofAirProps;
	std::ofstream ofParameters;
	std::ofstream ofExternals;
	std::ofstream ofModal;
	std::ofstream ofThermalNodes;
	std::ofstream ofThermalElements;
	std::ofstream ofPlates;
	std::ofstream ofGravity;
	std::ofstream ofDofStats;
	std::ofstream ofDriveCallers;
	std::ofstream ofTraces;

	int iCurrWidth;
	int iCurrPrecision;
	int nCurrRestartFile;

	// private because we know we're using valid out index
	bool IsOpen(int out) const;
	bool UseDefaultPrecision(int out) const;
	bool UseScientific(int out) const;

	bool UseText(int out) const;
	bool UseNetCDF(int out) const;

	// Pseudo-constructor
	void OutputHandler_int(void);

public:
	OutputHandler(void);

	OutputHandler(const char* sFName, int iExtNum = -1);

	void Init(const char* sFName, int iExtNum = -1);

	virtual ~OutputHandler(void);

	/* Aggiungere qui le funzioni che aprono i singoli stream */
	bool Open(const OutputHandler::OutFiles out);

	bool IsOpen(const OutputHandler::OutFiles out) const;
	bool UseDefaultPrecision(const OutputHandler::OutFiles out) const;
	bool UseScientific(const OutputHandler::OutFiles out) const;

	void SetText(const OutputHandler::OutFiles out);
	void ClearText(void);
	void ClearText(const OutputHandler::OutFiles out);
	bool UseText(const OutputHandler::OutFiles out) const;

	void SetNetCDF(const OutputHandler::OutFiles out);
	void ClearNetCDF(void);
	void ClearNetCDF(const OutputHandler::OutFiles out);
	bool UseNetCDF(const OutputHandler::OutFiles out) const;

	bool Close(const OutputHandler::OutFiles out);

	bool OutputOpen(void);
	bool RestartOpen(bool openResXSol = false);

	bool PartitionOpen(void);
	bool AdamsResOpen(void);
	bool AdamsCmdOpen(void);
	bool LogOpen(void);

	/* Aggiungere qui le funzioni che ritornano gli stream desiderati */
	inline std::ostream& Get(const OutputHandler::OutFiles f);

	inline std::ostream& Output(void) const;
	inline std::ostream& StrNodes(void) const;
	inline std::ostream& Electric(void) const;
	inline std::ostream& Abstract(void) const;
	inline std::ostream& Inertia(void) const;
	inline std::ostream& Joints(void) const;
	inline std::ostream& Forces(void) const;
	inline std::ostream& Beams(void) const;
	inline std::ostream& Rotors(void) const;
	inline std::ostream& Restart(void) const;
	inline std::ostream& RestartXSol(void) const;
	inline std::ostream& Aerodynamic(void) const;
	inline std::ostream& Hydraulic(void) const;
	inline std::ostream& PresNodes(void) const;
	inline std::ostream& Loadable(void) const;
	inline std::ostream& Genels(void) const;
	inline std::ostream& Partition(void) const;
	inline std::ostream& AdamsRes(void) const;
	inline std::ostream& AdamsCmd(void) const;
	inline std::ostream& AeroModals(void) const;
	inline std::ostream& ReferenceFrames(void) const;
	inline std::ostream& Log(void) const;
	inline std::ostream& AirProps(void) const;
	inline std::ostream& Parameters(void) const;
	inline std::ostream& Externals(void) const;
	inline std::ostream& Modal(void) const;
	inline std::ostream& ThermalNodes(void) const;
	inline std::ostream& ThermalElements(void) const;
	inline std::ostream& Plates(void) const;
	inline std::ostream& Gravity(void) const;
	inline std::ostream& DofStats(void) const;
	inline std::ostream& DriveCallers(void) const;
	inline std::ostream& Traces(void) const;

	inline int iW(void) const;
	inline int iP(void) const;

	void SetWidth(int iNewWidth);
	void SetPrecision(int iNewPrecision);

#ifdef USE_NETCDF
	inline NcFile* pGetBinFile(void) const;

	struct AttrVal {
		std::string attr;
		std::string val;
		AttrVal(void) { NO_OP; };
		AttrVal(const std::string& attr, const std::string& val) : attr(attr), val(val) { NO_OP; };
	};

	typedef std::vector<OutputHandler::AttrVal> AttrValVec;
	typedef std::vector<const NcDim *> NcDimVec;

	const NcDim *
	CreateDim(const std::string& name, integer size = -1);

	const NcDim *
	GetDim(const std::string& name) const;

	inline const NcDim* DimTime(void) const;
	inline const NcDim* DimV1(void) const;
	inline const NcDim* DimV3(void) const;

	NcVar *
	CreateVar(const std::string& name, NcType type,
		const AttrValVec& attrs, const NcDimVec& dims);

	NcVar *
	CreateVar(const std::string& name, const std::string& type);

	template <class T>
	NcVar *
	CreateVar(const std::string& name,
		const std::string& units, const std::string& description);

	NcVar *
	CreateRotationVar(const std::string& name_prefix,
		const std::string& name_postfix,
		OrientationDescription od,
		const std::string& description);
#endif /* USE_NETCDF */
}; /* End class OutputHandler */

#ifdef USE_NETCDF
template <class T>
NcVar *
OutputHandler::CreateVar(const std::string& name,
	const std::string& units, const std::string& description)
{
	AttrValVec attrs(3);
	NcDimVec dims(1);

	attrs[0] = AttrVal("units", units);
	attrs[2] = AttrVal("description", description);
	dims[0] = DimTime();

	NcType type;
	if (typeid(T) == typeid(integer)) {
		attrs[1] = AttrVal("type", "integer");
		type = ncLong;

	} else if (typeid(T) == typeid(doublereal)) {
		attrs[1] = AttrVal("type", "doublereal");
		type = ncDouble;

	} else if (typeid(T) == typeid(Vec3)) {
		attrs[1] = AttrVal("type", "Vec3");
		dims.resize(2);
		dims[1] = DimV3();
		type = ncDouble;

	} else {
		throw ErrGeneric(MBDYN_EXCEPT_ARGS);
	}

	return CreateVar(name, type, attrs, dims);
}

inline NcFile *
OutputHandler::pGetBinFile(void) const
{
	return m_pBinFile;
}

inline const NcDim *
OutputHandler::DimTime(void) const
{
	return m_DimTime;
}

inline const NcDim *
OutputHandler::DimV1(void) const
{
	return m_DimV1;
}

inline const NcDim *
OutputHandler::DimV3(void) const
{
	return m_DimV3;
}
#endif /* USE_NETCDF */

inline std::ostream&
OutputHandler::Get(const OutputHandler::OutFiles f)
{
	ASSERT(f > -1 && f < LASTFILE);
	ASSERT(IsOpen(f));
	return *(OutData[f].pof);
}

inline std::ostream&
OutputHandler::Output(void) const
{
#ifdef DEBUG_COUT
	return const_cast<std::ostream &>(cout);
#else
	ASSERT(IsOpen(OUTPUT));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofOutput));
#endif
}

inline std::ostream&
OutputHandler::StrNodes(void) const
{
	ASSERT(IsOpen(STRNODES));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofStrNodes));
}

inline std::ostream&
OutputHandler::Electric(void) const
{
	ASSERT(IsOpen(ELECTRIC));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofElectric));
}

inline std::ostream&
OutputHandler::ThermalNodes(void) const
{
	ASSERT(IsOpen(THERMALNODES));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofThermalNodes));
}

inline std::ostream&
OutputHandler::ThermalElements(void) const
{
	ASSERT(IsOpen(THERMALELEMENTS));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofThermalElements));
}

inline std::ostream&
OutputHandler::Abstract(void) const
{
	ASSERT(IsOpen(ABSTRACT));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofAbstract));
}

inline std::ostream&
OutputHandler::Inertia(void) const
{
	ASSERT(IsOpen(INERTIA));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofInertia));
}

inline std::ostream&
OutputHandler::Joints(void) const
{
	ASSERT(IsOpen(JOINTS));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofJoints));
}

inline std::ostream&
OutputHandler::Forces(void) const
{
	ASSERT(IsOpen(FORCES));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofForces));
}

inline std::ostream&
OutputHandler::Beams(void) const
{
	ASSERT(IsOpen(BEAMS));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofBeams));
}

inline std::ostream&
OutputHandler::Rotors(void) const
{
	ASSERT(IsOpen(ROTORS));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofRotors));
}

inline std::ostream&
OutputHandler::Restart(void) const
{
	ASSERT(IsOpen(RESTART));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofRestart));
}

inline std::ostream&
OutputHandler::RestartXSol(void) const
{
	ASSERT(IsOpen(RESTART));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofRestartXSol));
}

inline std::ostream&
OutputHandler::Aerodynamic(void) const
{
	ASSERT(IsOpen(AERODYNAMIC));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofAerodynamic));
}

inline std::ostream&
OutputHandler::Hydraulic(void) const
{
	ASSERT(IsOpen(HYDRAULIC));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofHydraulic));
}

inline std::ostream&
OutputHandler::PresNodes(void) const
{
	ASSERT(IsOpen(PRESNODES));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofPresNodes));
}

inline std::ostream&
OutputHandler::Loadable(void) const
{
	ASSERT(IsOpen(LOADABLE));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofLoadable));
}

inline std::ostream&
OutputHandler::Genels(void) const
{
	ASSERT(IsOpen(GENELS));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofGenels));
}

inline std::ostream&
OutputHandler::Partition(void) const
{
	ASSERT(IsOpen(PARTITION));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofPartition));
}

inline std::ostream&
OutputHandler::AdamsRes(void) const
{
	ASSERT(IsOpen(ADAMSRES));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofAdamsRes));
}

inline std::ostream&
OutputHandler::AdamsCmd(void) const
{
	ASSERT(IsOpen(ADAMSCMD));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofAdamsCmd));
}

inline std::ostream&
OutputHandler::AeroModals(void) const
{
	ASSERT(IsOpen(AEROMODALS));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofAeroModals));
}

inline std::ostream&
OutputHandler::ReferenceFrames(void) const
{
	ASSERT(IsOpen(REFERENCEFRAMES));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofReferenceFrames));
}

inline std::ostream&
OutputHandler::Log(void) const
{
#ifdef DEBUG_COUT
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(cout));
#else
	ASSERT(IsOpen(LOG));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofLog));
#endif
}

inline std::ostream&
OutputHandler::AirProps(void) const
{
	ASSERT(IsOpen(AIRPROPS));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofAirProps));
}

inline std::ostream&
OutputHandler::Parameters(void) const
{
	ASSERT(IsOpen(PARAMETERS));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofParameters));
}

inline std::ostream&
OutputHandler::Externals(void) const
{
	ASSERT(IsOpen(EXTERNALS));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofExternals));
}

inline std::ostream&
OutputHandler::Modal(void) const
{
	ASSERT(IsOpen(MODAL));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofModal));
}

inline std::ostream&
OutputHandler::Plates(void) const
{
	ASSERT(IsOpen(PLATES));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofPlates));
}

inline std::ostream&
OutputHandler::Gravity(void) const
{
	ASSERT(IsOpen(GRAVITY));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofGravity));
}

inline std::ostream&
OutputHandler::DofStats(void) const
{
	ASSERT(IsOpen(DOFSTATS));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofDofStats));
}

inline std::ostream&
OutputHandler::DriveCallers(void) const
{
	ASSERT(IsOpen(DRIVECALLERS));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofDriveCallers));
}

inline std::ostream&
OutputHandler::Traces(void) const
{
	ASSERT(IsOpen(TRACES));
	return const_cast<std::ostream &>(dynamic_cast<const std::ostream &>(ofTraces));
}

inline int
OutputHandler::iW(void) const
{
	return iCurrWidth;
}

inline int
OutputHandler::iP(void) const
{
	return iCurrPrecision;
}

/* OutputHandler - end */


/* ToBeOutput - begin */

const flag fDefaultOut = 1;

class ToBeOutput {
public:
	enum {
		OUTPUT = 0x1U,

		// use OUTPUT_MASK to isolate public output flags
		OUTPUT_MASK = 0xFU,

		// reserve values up to OUTPUT_PRIVATE for public use;
		// reserved output flags can start from OUTPUT_PRIVATE up
		OUTPUT_PRIVATE = 0x10U,

		// use OUTPUT_PRIVATE_MASK to isolate private output flags
		OUTPUT_PRIVATE_MASK = ~OUTPUT_MASK
	};

protected:
	flag fOutput;

public:
	ToBeOutput(flag fOut = fDefaultOut);
	virtual ~ToBeOutput(void);

	virtual void OutputPrepare(OutputHandler &OH);

	/* Regular output */
	virtual void Output(OutputHandler& OH) const;

	/* Output of perturbed solution (modes ...) */
	virtual void Output(OutputHandler& OH,
	const VectorHandler& X, const VectorHandler& XP) const;

	virtual flag fToBeOutput(void) const;
	virtual void SetOutputFlag(flag f = flag(1));
};

/* ToBeOutput - end */

class Traceable {
public:
	enum {
		TRACE 				= 0x01U,
		TRACE_PUBLIC_MASK	= 0x0FU,
		TRACE_PRIVATE		= 0x10U,
		TRACE_PRIVATE_MASK	= ~TRACE_PUBLIC_MASK
	};

	Traceable(flag fTrace = 0);
	virtual ~Traceable(void);

	virtual void Trace(OutputHandler& OH) const=0;
	virtual flag fToBeTraced(void) const;
	virtual void SetTraceFlag(flag f = TRACE);

private:
	flag fTrace;
};

#endif /* OUTPUT_H */

