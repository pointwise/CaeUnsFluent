/****************************************************************************
 *
 * (C) 2021 Cadence Design Systems, Inc. All rights reserved worldwide.
 *
 * This sample source code is not supported by Cadence Design Systems, Inc.
 * It is provided freely for demonstration purposes only.
 * SEE THE WARRANTY DISCLAIMER AT THE BOTTOM OF THIS FILE.
 *
 ***************************************************************************/
/****************************************************************************
 *
 * Pointwise Plugin utility functions
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
    { "Interior", 14 },             // shadow/non-inflated
    { "Pressure Inlet", 4 },
    { "Intake Fan", 4 },
    { "Inlet Vent", 4 },
    { "Pressure Outlet", 5 },
    { "Exhaust Fan", 5 },
    { "Outlet Vent", 5 },
    { "Symmetry", 7 },
    { "Pressure Far Field", 9 },
    { "Velocity Inlet", 10 },
    { "Porous Jump", 14 },          // shadow/non-inflated
    { "Fan", 14 },                  // shadow/non-inflated
    { "Radiator", 14 },             // shadow/non-inflated
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

#endif /* _RTCAEPSUPPORTDATA_H_ */

/****************************************************************************
 *
 * This file is licensed under the Cadence Public License Version 1.0 (the
 * "License"), a copy of which is found in the included file named "LICENSE",
 * and is distributed "AS IS." TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE
 * LAW, CADENCE DISCLAIMS ALL WARRANTIES AND IN NO EVENT SHALL BE LIABLE TO
 * ANY PARTY FOR ANY DAMAGES ARISING OUT OF OR RELATING TO USE OF THIS FILE.
 * Please see the License for the full text of applicable terms.
 *
 ****************************************************************************/
