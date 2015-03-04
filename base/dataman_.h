/* $Header: /var/cvs/mbdyn/mbdyn/mbdyn-1.0/mbdyn/base/dataman_.h,v 1.18 2014/07/22 19:19:17 masarati Exp $ */
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

/* File di include solo per i dataman*.cc */

#ifndef DATAMAN__H
#define DATAMAN__H

extern void SetDrvHdl(DriveHandler*);
extern const flag fDefaultSkipInitialJointAssembly;
extern const doublereal dDefaultInitialStiffness;
extern const flag fDefaultOmegaRotates;
extern const doublereal dDefaultInitialAssemblyTol;
extern const integer iDefaultMaxInitialIterations;

#endif /* DATAMAN__H */
