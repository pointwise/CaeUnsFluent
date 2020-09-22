/****************************************************************************
 *
 * Pointwise Plugin utility functions
 *
 * Proprietary software product of Pointwise, Inc.
 * Copyright (c) 1995-2020 Pointwise, Inc.
 * All rights reserved.
 *
 ***************************************************************************/

#ifndef _RTCAEPSUPPORTDATA_H_
#define _RTCAEPSUPPORTDATA_H_

/*! \cond */

/*------------------------------------*/
/* CaeUnsFluent format item setup data */
/*------------------------------------*/
CAEP_BCINFO CaeUnsFluentBCInfo[] = {
    { "Wall", 3 },
    { "Interior", 14 },
    { "Pressure Inlet", 4 },
    { "Intake Fan", 4 },
    { "Inlet Vent", 4 },
    { "Pressure Outlet", 5 },
    { "Exhaust Fan", 5 },
    { "Outlet Vent", 5 },
    { "Symmetry", 7 },
    { "Pressure Far Field", 9 },
    { "Velocity Inlet", 10 },
    { "Porous Jump", 14 },
    { "Fan", 14 },
    { "Radiator", 14 },
    { "Mass Flow Inlet", 20 },
    { "Recirculation Outlet", 20 },
    { "Recirculation Inlet", 20 },
    { "Interface", 24 },
    { "Outflow", 36 },
    { "Axis", 37 },
};
/*------------------------------------*/
CAEP_VCINFO CaeUnsFluentVCInfo[] = {
    { "Fluid", 1 },
    { "Solid", 2 },
};
/*------------------------------------*/
const char *CaeUnsFluentFileExt[] = {
    "cas"
};
/*! \endcond */



/************************************************************************/
/*! \file
\brief Defines Support Data for the CAEP_RTITEM Array

The file \sf{%rtCaepSupportData.h} defines and initilizes the support data for
the global CAEP_RTITEM \ref caepRtItem[] array. The CAE Plugin SDK uses this data
to implement the functions and behaviors required by the \ref DOXGRP_APICAEP.
If you want to see the SDK implementation details, look in the
\sf{/shared/CAEP/apiCAEP.cxx} file.

The SDK file \sf{/shared/CAEP/apiCAEP.cxx} includes \sf{%rtCaepSupportData.h}
prior to the declaration of the \ref caepRtItem[] array as shown below.
\par
\dontinclude apiCAEP.cxx
\skip "rtCaepSupportData.h"
\until ARRAYSIZE(caepRtItem);

When copied from the \sf{src/plugins/templates/CAEP/} folder to your plugins
project folder, \sf{%rtCaepSupportData.h} will contain the support data needed
for the 3 example CAEP_RTITEM array items. This example support data must be
culled and edited as needed for your plugin's implementation.

The support data used by the CAE Plugin SDK includes the following:

\li Array of valid Boundary Conditions (CAEP_BCINFO) definitions.
\li Array of valid Volume Conditions (CAEP_VCINFO) definitions.
\li Array of valid File Extension definitions.

These support arrays are referenced in rtCaepInitItems.h to initialize the
corresponding data members.

\par Example Support Data Usage

The code segments below show how the example support data is implemented by
the SDK in the \sf{%rtCaepSupportData.h} and rtCaepInitItems.h template files.

The example support data declaration and initialization in
\sf{%rtCaepSupportData.h}:
\par
\dontinclude rtCaepSupportData.h
\skip CaeUnsFluentBCInfo[]
\until "xml"
\skipline };

The support data referenced as initilizers in rtCaepInitItems.h:
\par
\dontinclude rtCaepInitItems.h
\skip == CAEP_BCINFO*
\until ARRAYSIZE(CaeUnsFluentFileExt)
*/

#endif /* _RTCAEPSUPPORTDATA_H_ */
