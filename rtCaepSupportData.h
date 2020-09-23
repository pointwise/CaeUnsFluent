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
* DISCLAIMER:
* TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, POINTWISE DISCLAIMS
* ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED
* TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE, WITH REGARD TO THIS SCRIPT. TO THE MAXIMUM EXTENT PERMITTED
* BY APPLICABLE LAW, IN NO EVENT SHALL POINTWISE BE LIABLE TO ANY PARTY
* FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
* WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF
* BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE
* USE OF OR INABILITY TO USE THIS SCRIPT EVEN IF POINTWISE HAS BEEN
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGES AND REGARDLESS OF THE
* FAULT OR NEGLIGENCE OF POINTWISE.
*
***************************************************************************/
