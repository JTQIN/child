/****************************************************************************\
**   Header file for defining global constants
**   $Id: Definitions.h,v 1.13 1999-02-01 21:47:09 gtucker Exp $
\****************************************************************************/

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

/** DEFINITIONS *************************************************************/
#define VERSION "Beta 1.0"
#define TRUE 1
#define FALSE 0
//#define kMaxNameLength 80
//#define kCommentMark '#'
//#define kTimeLineMark ' '
#define kUniformGrid 0     /*method of grid construction*/
#define kPerturbedGrid 1
#define kRandomGrid 2
#define kCornerOutlet 0   /*type of open boundary*/
#define kOpenSide 1
#define kOppositeSidesOpen 2
#define kAllSidesOpen 3
#define kSpecifyOutlet 4
#define kClosedBoundary 1
#define kOpenBoundary 2
#define kNonBoundary 0
#define kFlowAllowed 1
#define kFlowNotAllowed 0
#define kDetachmentLimited      1
#define kDetachLimThreshold     2
#define kTransportLimited       3
#define kTransLimThreshold      4
#define kBedrockAlluvial        5
#define kMeanderNode 1
#define kNonMeanderNode 0
#define RHO 1000.0      /* Density of water (kg/m^3) */
#define RHOSED 2650. /* density of sediment, [kg/m^3] */
#define GRAV 9.81       /* Gravitational acceleration, m/s^2 */
#define POROSITY 0.3        /* porosity of sediment on bed */
#define VISC .00000112      /* viscosity of water [m^2/s] */

// Macros
#define ROUND(x)    (int)(x+0.5)
#define SIGN(x)     ( x>0 ? 1 : 0 )

// Redefine floats to be double precision (8 byte) for compatibility
// with meandering routine
//#define float double

#endif
