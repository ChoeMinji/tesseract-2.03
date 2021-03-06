/******************************************************************************
 **	Filename:    mfoutline.c
 **	Purpose:     Interface to outline struct used for extracting features
 **	Author:      Dan Johnson
 **	History:     Thu May 17 08:14:18 1990, DSJ, Created.
 **
 **	(c) Copyright Hewlett-Packard Company, 1988.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 ******************************************************************************/
/**----------------------------------------------------------------------------
          Include Files and Type Defines
----------------------------------------------------------------------------**/
#include "clusttool.h"           //If remove you get cought in a loop somewhere
#include "emalloc.h"
#include "mfoutline.h"
#include "debug.h"
#include "hideedge.h"
#include "blobs.h"
#include "const.h"
#include "mfx.h"

#include <math.h>
#include <stdio.h>

#define MIN_INERTIA (0.00001)

/**----------------------------------------------------------------------------
          Private Function Prototypes
----------------------------------------------------------------------------**/
/*
#if defined(__STDC__) || defined(__cplusplus)
# define _ARGS(s) s
#else
# define _ARGS(s) ()
#endif*/

/* /users/danj/wiseowl/src/danj/microfeatures/mfoutline.c
void ChangeDirection
  _ARGS((MFOUTLINE Start,
  MFOUTLINE End,
  DIRECTION Direction));

void CharNormalizeOutline
  _ARGS((MFOUTLINE Outline,
  OUTLINE_STATS *OutlineStats));

void ComputeDirection
  _ARGS((MFEDGEPT *Start,
  MFEDGEPT *Finish,
  FLOAT32 MinSlope,
  FLOAT32 MaxSlope));

void FinishOutlineStats
  _ARGS((OUTLINE_STATS *OutlineStats));

void InitOutlineStats
  _ARGS((OUTLINE_STATS *OutlineStats));

MFOUTLINE NextDirectionChange
  _ARGS((MFOUTLINE EdgePoint));

void UpdateOutlineStats
  _ARGS((OUTLINE_STATS *OutlineStats,
  FLOAT32 x1,
  FLOAT32 y1,
  FLOAT32 x2,
  FLOAT32 y2));

#undef _ARGS
*/
/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
/* center of current blob being processed - used when "unexpanding"
  expanded blobs */
static TPOINT BlobCenter;

/* control knobs used to control normalization of outlines */
make_int_var (NormMethod, character, MakeNormMethod,
15, 10, SetNormMethod, "Normalization Method   ...")
/* PREV DEFAULT "baseline" */
make_float_var (CharNormRange, 0.2, MakeCharNormRange,
15, 11, SetCharNormRange, "Character Normalization Range ...")
make_float_var (MinNormScaleX, 0.0, MakeMinNormScaleX,
15, 12, SetMinNormScaleX, "Min char x-norm scale ...")
/* PREV DEFAULT 0.1 */
make_float_var (MaxNormScaleX, 0.325, MakeMaxNormScaleX,
15, 13, SetMaxNormScaleX, "Max char x-norm scale ...")
/* PREV DEFAULT 0.3 */
make_float_var (MinNormScaleY, 0.0, MakeMinNormScaleY,
15, 14, SetMinNormScaleY, "Min char y-norm scale ...")
/* PREV DEFAULT 0.1 */
make_float_var (MaxNormScaleY, 0.325, MakeMaxNormScaleY,
15, 15, SetMaxNormScaleY, "Max char y-norm scale ...")
/* PREV DEFAULT 0.3 */
/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
void ComputeBlobCenter(TBLOB *Blob, TPOINT *BlobCenter) {
/*
 **	Parameters:
 **		Blob		blob to compute centerpoint of
 **		BlobCenter	data struct to place results in
 **	Globals: none
 **	Operation:
 **		This routine computes the center point of the specified
 **		blob using the bounding box of all top level outlines in the
 **		blob.  The center point is computed in a coordinate system
 **		which is scaled up by VECSCALE from the page coordinate
 **		system.
 **	Return: none
 **	Exceptions: none
 **	History: Fri Sep  8 10:45:39 1989, DSJ, Created.
 */
  TPOINT TopLeft;
  TPOINT BottomRight;

  blob_bounding_box(Blob, &TopLeft, &BottomRight);

  BlobCenter->x = ((TopLeft.x << VECSCALE) + (BottomRight.x << VECSCALE)) / 2;
  BlobCenter->y = ((TopLeft.y << VECSCALE) + (BottomRight.y << VECSCALE)) / 2;

}                                /* ComputeBlobCenter */


/*---------------------------------------------------------------------------*/
LIST ConvertBlob(TBLOB *Blob) {
/*
 **	Parameters:
 **		Blob	blob to be converted
 **	Globals: none
 **	Operation: Convert Blob into a list of outlines.
 **	Return: List of outlines representing blob.
 **	Exceptions: none
 **	History: Thu Dec 13 15:40:17 1990, DSJ, Created.
 */
  LIST ConvertedOutlines = NIL;

  if (Blob != NULL) {
    SettupBlobConversion(Blob);  //ComputeBlobCenter (Blob, &BlobCenter);
    ConvertedOutlines = ConvertOutlines (Blob->outlines,
      ConvertedOutlines, outer);
  }

  return (ConvertedOutlines);
}                                /* ConvertBlob */


/*---------------------------------------------------------------------------*/
MFOUTLINE ConvertOutline(TESSLINE *Outline) {
/*
 **	Parameters:
 **		Outline		outline to be converted
 **	Globals:
 **		BlobCenter	pre-computed center of current blob
 **	Operation:
 **		This routine converts the specified outline into a special
 **		data structure which is used for extracting micro-features.
 **		If the outline has been pre-normalized by the splitter,
 **		then it is assumed to be in expanded form and all we must
 **		do is copy the points.  Otherwise,
 **		if the outline is expanded, then the expanded form is used
 **		and the coordinates of the points are returned to page
 **		coordinates using the global variable BlobCenter and the
 **		scaling factor REALSCALE.  If the outline is not expanded,
 **		then the compressed form is used.
 **	Return: Outline converted into special micro-features format.
 **	Exceptions: none
 **	History: 8/2/89, DSJ, Created.
 **		9/8/89, DSJ, Added ability to convert expanded blobs.
 **		1/11/90, DSJ, Changed to use REALSCALE instead of VECSCALE
 **					to eliminate round-off problems.
 **		2/21/91, DSJ, Added ability to work with pre-normalized
 **					blobs.
 **		4/30/91, DSJ, Added concept of "hidden" segments.
 */
  register BYTEVEC *Vector;
  TPOINT Position;
  TPOINT StartPosition;
  MFEDGEPT *NewPoint;
  MFOUTLINE MFOutline = NIL;
  EDGEPT *EdgePoint;
  EDGEPT *StartPoint;
  EDGEPT *NextPoint;

  if (Outline == NULL ||
    (Outline->compactloop == NULL && Outline->loop == NULL))
    return (MFOutline);

                                 /* have outlines been prenormalized */
  if (is_baseline_normalized ()) {
    StartPoint = Outline->loop;
    EdgePoint = StartPoint;
    do {
      NextPoint = EdgePoint->next;

      /* filter out duplicate points */
      if (EdgePoint->pos.x != NextPoint->pos.x ||
      EdgePoint->pos.y != NextPoint->pos.y) {
        NewPoint = NewEdgePoint ();
        ClearMark(NewPoint);
        IsHidden (NewPoint) = is_hidden_edge (EdgePoint) ? TRUE : FALSE;
        XPositionOf (NewPoint) = EdgePoint->pos.x;
        YPositionOf (NewPoint) = EdgePoint->pos.y;
        MFOutline = push (MFOutline, NewPoint);
      }
      EdgePoint = NextPoint;
    }
    while (EdgePoint != StartPoint);
  }
                                 /* use compressed version of outline */
  else if (Outline->loop == NULL) {
    Xof (Position) = Xof (StartPosition) = Outline->start.x;
    Yof (Position) = Yof (StartPosition) = Outline->start.y;
    Vector = Outline->compactloop;
    do {
      if (Vector->dx != 0 || Vector->dy != 0) {
        NewPoint = NewEdgePoint ();
        ClearMark(NewPoint);
                                 /* all edges are visible */
        IsHidden (NewPoint) = FALSE;
        CopyPoint (Position, PositionOf (NewPoint));
        MFOutline = push (MFOutline, NewPoint);
      }
      Xof (Position) += Vector->dx;
      Yof (Position) += Vector->dy;
      Vector++;
    }
    while ((Xof (Position) != Xof (StartPosition)) ||
      (Yof (Position) != Yof (StartPosition)));
  }
  else {                         /* use expanded version of outline */
    StartPoint = Outline->loop;
    EdgePoint = StartPoint;
    do {
      NextPoint = EdgePoint->next;

      /* filter out duplicate points */
      if (EdgePoint->pos.x != NextPoint->pos.x ||
      EdgePoint->pos.y != NextPoint->pos.y) {
        NewPoint = NewEdgePoint ();
        ClearMark(NewPoint);
        IsHidden (NewPoint) = is_hidden_edge (EdgePoint) ? TRUE : FALSE;
        XPositionOf (NewPoint) =
          (EdgePoint->pos.x + BlobCenter.x) / REALSCALE;
        YPositionOf (NewPoint) =
          (EdgePoint->pos.y + BlobCenter.y) / REALSCALE;
        MFOutline = push (MFOutline, NewPoint);
      }
      EdgePoint = NextPoint;
    }
    while (EdgePoint != StartPoint);
  }

  MakeOutlineCircular(MFOutline);
  return (MFOutline);

}                                /* ConvertOutline */


/*---------------------------------------------------------------------------*/
LIST ConvertOutlines(TESSLINE *Outline,
                     LIST ConvertedOutlines,
                     OUTLINETYPE OutlineType) {
/*
 **	Parameters:
 **		Outline			first outline to be converted
 **		ConvertedOutlines	list to add converted outlines to
 **		OutlineType		are the outlines outer or holes?
 **	Globals: none
 **	Operation:
 **              This routine converts all given outlines into a new format.
 **              of outlines.  Outline points to a list of the top level
 **		outlines to be converted.  The children of these outlines
 **		are also recursively converted.  All converted outlines
 **		are added to ConvertedOutlines.  This is a list of outlines,
 **		one for each outline that was converted.
 **	Return: Updated list of converted outlines.
 **	Exceptions: none
 **	History: Thu Dec 13 15:57:38 1990, DSJ, Created.
 */
  MFOUTLINE MFOutline;

  while (Outline != NULL) {
    if (Outline->child != NULL) {
      if (OutlineType == outer)
        ConvertedOutlines = ConvertOutlines (Outline->child,
          ConvertedOutlines, hole);
      else
        ConvertedOutlines = ConvertOutlines (Outline->child,
          ConvertedOutlines, outer);
    }

    MFOutline = ConvertOutline (Outline);
    ConvertedOutlines = push (ConvertedOutlines, MFOutline);
    Outline = Outline->next;
  }
  return (ConvertedOutlines);
}                                /* ConvertOutlines */


/*---------------------------------------------------------------------------*/
void ComputeOutlineStats(LIST Outlines, OUTLINE_STATS *OutlineStats) {
/*
 **	Parameters:
 **		Outlines	list of outlines to compute stats for
 **		OutlineStats	place to put results
 **	Globals: none
 **	Operation: This routine computes several statistics about the outlines
 **		in Outlines.  These statistics are usually used to perform
 **		anistropic normalization of all of the outlines.  The
 **		statistics generated are:
 **			first moments about x and y axes
 **			total length of all outlines
 **			center of mass of all outlines
 **			second moments about center of mass axes
 **			radius of gyration about center of mass axes
 **	Return: none (results are returned in OutlineStats)
 **	Exceptions: none
 **	History: Fri Dec 14 08:32:03 1990, DSJ, Created.
 */
  MFOUTLINE Outline;
  MFOUTLINE EdgePoint;
  MFEDGEPT *Current;
  MFEDGEPT *Last;

  InitOutlineStats(OutlineStats);
  iterate(Outlines) {
    Outline = (MFOUTLINE) first_node (Outlines);

    Last = PointAt (Outline);
    Outline = NextPointAfter (Outline);
    EdgePoint = Outline;
    do {
      Current = PointAt (EdgePoint);

      UpdateOutlineStats (OutlineStats,
        XPositionOf (Last), YPositionOf (Last),
        XPositionOf (Current), YPositionOf (Current));

      Last = Current;
      EdgePoint = NextPointAfter (EdgePoint);
    }
    while (EdgePoint != Outline);
  }
  FinishOutlineStats(OutlineStats);

}                                /* ComputeOutlineStats */


/*---------------------------------------------------------------------------*/
void FilterEdgeNoise(MFOUTLINE Outline, FLOAT32 NoiseSegmentLength) {
/*
 **	Parameters:
 **		Outline		outline to be filtered
 **		NoiseSegmentLength	maximum length of a "noise" segment
 **	Globals: none
 **	Operation: Filter out noise from the specified outline.  This is
 **		done by changing the direction of short segments of the
 **		outline to the same direction as the preceding outline
 **		segment.
 **	Return: none
 **	Exceptions: none
 **	History: Fri May  4 10:23:45 1990, DSJ, Created.
 */
  MFOUTLINE Current;
  MFOUTLINE Last;
  MFOUTLINE First;
  FLOAT32 Length;
  int NumFound = 0;
  DIRECTION DirectionOfFirst = north;

  if (DegenerateOutline (Outline))
    return;

  /* find 2 segments of different orientation which are long enough to
     not be filtered.  If two cannot be found, leave the outline unchanged. */
  First = NextDirectionChange (Outline);
  Last = First;
  do {
    Current = NextDirectionChange (Last);
    Length = DistanceBetween (PositionOf (PointAt (Current)),
      PositionOf (PointAt (Last)));
    if (Length >= NoiseSegmentLength) {
      if (NumFound == 0) {
        NumFound = 1;
        DirectionOfFirst = DirectionOf (PointAt (Last));
      }
      else if (DirectionOfFirst != DirectionOf (PointAt (Last)))
        break;
    }
    Last = Current;
  }
  while (Last != First);
  if (Current == Last)
    return;

  /* find each segment and filter it out if it is too short.  Note that
     the above code guarantees that the initial direction change will
     not be removed, therefore the loop will terminate. */
  First = Last;
  do {
    Current = NextDirectionChange (Last);
    Length = DistanceBetween (PositionOf (PointAt (Current)),
      PositionOf (PointAt (Last)));
    if (Length < NoiseSegmentLength)
      ChangeDirection (Last, Current, PreviousDirectionOf (PointAt (Last)));

    Last = Current;
  }
  while (Last != First);

}                                /* FilterEdgeNoise */


/*---------------------------------------------------------------------------*/
void FindDirectionChanges(MFOUTLINE Outline,
                          FLOAT32 MinSlope,
                          FLOAT32 MaxSlope) {
/*
 **	Parameters:
 **		Outline		micro-feature outline to analyze
 **		MinSlope	controls "snapping" of segments to horizontal
 **		MaxSlope	controls "snapping" of segments to vertical
 **	Globals: none
 **	Operation:
 **		This routine searches thru the specified outline, computes
 **		a slope for each vector in the outline, and marks each
 **		vector as having one of the following directions:
 **			N, S, E, W, NE, NW, SE, SW
 **		This information is then stored in the outline and the
 **		outline is returned.
 **	Return: none
 **	Exceptions: none
 **	History: 7/21/89, DSJ, Created.
 */
  MFEDGEPT *Current;
  MFEDGEPT *Last;
  MFOUTLINE EdgePoint;

  if (DegenerateOutline (Outline))
    return;

  Last = PointAt (Outline);
  Outline = NextPointAfter (Outline);
  EdgePoint = Outline;
  do {
    Current = PointAt (EdgePoint);
    ComputeDirection(Last, Current, MinSlope, MaxSlope);

    Last = Current;
    EdgePoint = NextPointAfter (EdgePoint);
  }
  while (EdgePoint != Outline);

}                                /* FindDirectionChanges */


/*---------------------------------------------------------------------------*/
void FreeMFOutline(void *arg) {  //MFOUTLINE                             Outline)
/*
 **	Parameters:
 **		Outline		micro-feature outline to be freed
 **	Globals: none
 **	Operation:
 **		This routine deallocates all of the memory consumed by
 **		a micro-feature outline.
 **	Return: none
 **	Exceptions: none
 **	History: 7/27/89, DSJ, Created.
 */
  MFOUTLINE Start;
  MFOUTLINE Outline = (MFOUTLINE) arg;

  /* break the circular outline so we can use std. techniques to deallocate */
  Start = rest (Outline);
  set_rest(Outline, NIL);
  while (Start != NULL) {
    c_free_struct (first_node (Start), sizeof (MFEDGEPT), "MFEDGEPT");
    Start = pop (Start);
  }

}                                /* FreeMFOutline */


/*---------------------------------------------------------------------------*/
void FreeOutlines(LIST Outlines) {
/*
 **	Parameters:
 **		Outlines	list of mf-outlines to be freed
 **	Globals: none
 **	Operation: Release all memory consumed by the specified list
 **		of outlines.
 **	Return: none
 **	Exceptions: none
 **	History: Thu Dec 13 16:14:50 1990, DSJ, Created.
 */
  destroy_nodes(Outlines, FreeMFOutline);
}                                /* FreeOutlines */


/*---------------------------------------------------------------------------*/
void InitMFOutlineVars() {
/*
 **	Parameters: none
 **	Globals: none
 **	Operation: This routine initializes the global control knobs for
 **		all routines in this file.
 **	Return: none
 **	Exceptions: none
 **	History: Fri Dec 14 10:50:12 1990, DSJ, Created.
 */
  MakeNormMethod();
  MakeCharNormRange();
  MakeMinNormScaleX();
  MakeMaxNormScaleX();
  MakeMinNormScaleY();
  MakeMaxNormScaleY();
}                                /* InitMFOutlineVars */


/*---------------------------------------------------------------------------*/
void MarkDirectionChanges(MFOUTLINE Outline) {
/*
 **	Parameters:
 **		Outline		micro-feature outline to analyze
 **	Globals: none
 **	Operation:
 **		This routine searches thru the specified outline and finds
 **		the points at which the outline changes direction.  These
 **		points are then marked as "extremities".  This routine is
 **		used as an alternative to FindExtremities().  It forces the
 **		endpoints of the microfeatures to be at the direction
 **		changes rather than at the midpoint between direction
 **		changes.
 **	Return: none
 **	Exceptions: none
 **	History: 6/29/90, DSJ, Created.
 */
  MFOUTLINE Current;
  MFOUTLINE Last;
  MFOUTLINE First;

  if (DegenerateOutline (Outline))
    return;

  First = NextDirectionChange (Outline);
  Last = First;
  do {
    Current = NextDirectionChange (Last);
    MarkPoint (PointAt (Current));
    Last = Current;
  }
  while (Last != First);

}                                /* MarkDirectionChanges */


/*---------------------------------------------------------------------------*/
MFEDGEPT *NewEdgePoint() {
/*
 **	Parameters: none
 **	Globals: none
 **	Operation:
 **		This routine allocates and returns a new edge point for
 **		a micro-feature outline.
 **	Return: New edge point.
 **	Exceptions: none
 **	History: 7/21/89, DSJ, Created.
 */
  return ((MFEDGEPT *) c_alloc_struct (sizeof (MFEDGEPT), "MFEDGEPT"));

}                                /* NewEdgePoint */


/*---------------------------------------------------------------------------*/
MFOUTLINE NextExtremity(MFOUTLINE EdgePoint) {
/*
 **	Parameters:
 **		EdgePoint	start search from this point
 **	Globals: none
 **	Operation:
 **		This routine returns the next point in the micro-feature
 **		outline that is an extremity.  The search starts after
 **		EdgePoint.  The routine assumes that the outline being
 **		searched is not a degenerate outline (i.e. it must have
 **		2 or more edge points).
 **	Return: Next extremity in the outline after EdgePoint.
 **	Exceptions: none
 **	History: 7/26/89, DSJ, Created.
 */
  EdgePoint = NextPointAfter (EdgePoint);
  while (NotExtremity (PointAt (EdgePoint)))
    EdgePoint = NextPointAfter (EdgePoint);

  return (EdgePoint);

}                                /* NextExtremity */


/*---------------------------------------------------------------------------*/
void NormalizeOutline(MFOUTLINE Outline,
                      LINE_STATS *LineStats,
                      FLOAT32 XOrigin) {
/*
 **	Parameters:
 **		Outline		outline to be normalized
 **		LineStats	statistics for text line normalization
 **		XOrigin		x-origin of text
 **	Globals: none
 **	Operation:
 **		This routine normalizes the coordinates of the specified
 **		outline so that the outline is deskewed down to the
 **		baseline, translated so that x=0 is at XOrigin, and scaled
 **		so that the height of a character cell from descender to
 **		ascender is 1.  Of this height, 0.25 is for the descender,
 **		0.25 for the ascender, and 0.5 for the x-height.  The
 **		y coordinate of the baseline is 0.
 **	Return: none
 **	Exceptions: none
 **	History: 8/2/89, DSJ, Created.
 **		10/23/89, DSJ, Added ascender/descender stretching.
 **		11/89, DSJ, Removed ascender/descender stretching.
 */
  MFEDGEPT *Current;
  MFOUTLINE EdgePoint;
  FLOAT32 ScaleFactor;
  FLOAT32 AscStretch;
  FLOAT32 DescStretch;

  if (Outline != NIL) {
    ScaleFactor = ComputeScaleFactor (LineStats);
    AscStretch = 1.0;
    DescStretch = 1.0;

    EdgePoint = Outline;
    do {
      Current = PointAt (EdgePoint);

      YPositionOf (Current) = ScaleFactor *
        (YPositionOf (Current) -
        BaselineAt (LineStats, XPositionOf (Current)));

      if (YPositionOf (Current) > NORMAL_X_HEIGHT)
        YPositionOf (Current) = NORMAL_X_HEIGHT +
          (YPositionOf (Current) - NORMAL_X_HEIGHT) / AscStretch;

      else if (YPositionOf (Current) < NORMAL_BASELINE)
        YPositionOf (Current) = NORMAL_BASELINE +
            (YPositionOf (Current) - NORMAL_BASELINE) / DescStretch;

      XPositionOf (Current) = ScaleFactor *
        (XPositionOf (Current) - XOrigin);

      EdgePoint = NextPointAfter (EdgePoint);
    }
    while (EdgePoint != Outline);
  }
}                                /* NormalizeOutline */


/*---------------------------------------------------------------------------*/
void NormalizeOutlines(LIST Outlines,
                       LINE_STATS *LineStats,
                       FLOAT32 *XScale,
                       FLOAT32 *YScale) {
/*
 **	Parameters:
 **		Outlines	list of outlines to be normalized
 **		LineStats	statistics for text line normalization
 **		XScale		x-direction scale factor used by routine
 **		YScale		y-direction scale factor used by routine
 **	Globals:
 **		NormMethod	method being used for normalization
 **		CharNormRange	map radius of gyration to this value
 **	Operation: This routine normalizes every outline in Outlines
 **		according to the currently selected normalization method.
 **		It also returns the scale factors that it used to do this
 **		scaling.  The scale factors returned represent the x and
 **		y sizes in the normalized coordinate system that correspond
 **		to 1 pixel in the original coordinate system.
 **	Return: none (Outlines are changed and XScale and YScale are updated)
 **	Exceptions: none
 **	History: Fri Dec 14 08:14:55 1990, DSJ, Created.
 */
  MFOUTLINE Outline;
  OUTLINE_STATS OutlineStats;
  FLOAT32 BaselineScale;

  switch (NormMethod) {
    case character:
      ComputeOutlineStats(Outlines, &OutlineStats);

      /* limit scale factor to avoid overscaling small blobs (.,`'),
         thin blobs (l1ift), and merged blobs */
      *XScale = *YScale = BaselineScale = ComputeScaleFactor (LineStats);
      *XScale *= OutlineStats.Ry;
      *YScale *= OutlineStats.Rx;
      if (*XScale < MinNormScaleX)
        *XScale = MinNormScaleX;
      if (*YScale < MinNormScaleY)
        *YScale = MinNormScaleY;
      if (*XScale > MaxNormScaleX && *YScale <= MaxNormScaleY)
        *XScale = MaxNormScaleX;
      *XScale = CharNormRange * BaselineScale / *XScale;
      *YScale = CharNormRange * BaselineScale / *YScale;

      iterate(Outlines) {
        Outline = (MFOUTLINE) first_node (Outlines);
        CharNormalizeOutline (Outline,
          OutlineStats.x, OutlineStats.y,
          *XScale, *YScale);
      }
      break;

    case baseline:
      iterate(Outlines) {
        Outline = (MFOUTLINE) first_node (Outlines);
        NormalizeOutline (Outline, LineStats, 0.0);
      }
      *XScale = *YScale = ComputeScaleFactor (LineStats);
      break;
  }
}                                /* NormalizeOutlines */


/*---------------------------------------------------------------------------*/
void SettupBlobConversion(TBLOB *Blob) {
/*
 **	Parameters:
 **		Blob		blob that is to be converted
 **	Globals:
 **		BlobCenter	center of blob to be converted
 **	Operation: Compute the center of the blob's bounding box and save
 **		it in a global variable.  This routine must be called before
 **		any calls to ConvertOutline.  It must be called once per
 **		blob.
 **	Return: none
 **	Exceptions: none
 **	History: Thu May 17 11:06:17 1990, DSJ, Created.
 */
  ComputeBlobCenter(Blob, &BlobCenter);

}                                /* SettupBlobConversion */


/*---------------------------------------------------------------------------*/
void SmearExtremities(MFOUTLINE Outline, FLOAT32 XScale, FLOAT32 YScale) {
/*
 **	Parameters:
 **		Outline		outline whose extremities are to be smeared
 **		XScale		factor used to normalize outline in x dir
 **		YScale		factor used to normalize outline in y dir
 **	Globals: none
 **	Operation:
 **		This routine smears the extremities of the specified outline.
 **		It does this by adding a random number between
 **		-0.5 and 0.5 pixels (that is why X/YScale are needed) to
 **		the x and y position of the point.  This is done so that
 **		the discrete nature of the original scanned image does not
 **		affect the statistical clustering used during training.
 **	Return: none
 **	Exceptions: none
 **	History: 1/11/90, DSJ, Created.
 */
  MFEDGEPT *Current;
  MFOUTLINE EdgePoint;
  FLOAT32 MinXSmear;
  FLOAT32 MaxXSmear;
  FLOAT32 MinYSmear;
  FLOAT32 MaxYSmear;

  if (Outline != NIL) {
    MinXSmear = -0.5 * XScale;
    MaxXSmear = 0.5 * XScale;
    MinYSmear = -0.5 * YScale;
    MaxYSmear = 0.5 * YScale;
    EdgePoint = Outline;
    do {
      Current = PointAt (EdgePoint);
      if (IsExtremity (Current)) {
        XPositionOf (Current) +=
          UniformRandomNumber(MinXSmear, MaxXSmear);
        YPositionOf (Current) +=
          UniformRandomNumber(MinYSmear, MaxYSmear);
      }

      EdgePoint = NextPointAfter (EdgePoint);
    }
    while (EdgePoint != Outline);
  }
}                                /* SmearExtremities */


/**----------------------------------------------------------------------------
              Private Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
void ChangeDirection(MFOUTLINE Start, MFOUTLINE End, DIRECTION Direction) {
/*
 **	Parameters:
 **		Start, End	defines segment of outline to be modified
 **		Direction	new direction to assign to segment
 **	Globals: none
 **	Operation: Change the direction of every vector in the specified
 **		outline segment to Direction.  The segment to be changed
 **		starts at Start and ends at End.  Note that the previous
 **		direction of End must also be changed to reflect the
 **		change in direction of the point before it.
 **	Return: none
 **	Exceptions: none
 **	History: Fri May  4 10:42:04 1990, DSJ, Created.
 */
  MFOUTLINE Current;

  for (Current = Start; Current != End; Current = NextPointAfter (Current))
    DirectionOf (PointAt (Current)) = Direction;

  PreviousDirectionOf (PointAt (End)) = Direction;

}                                /* ChangeDirection */


/*---------------------------------------------------------------------------*/
void CharNormalizeOutline(MFOUTLINE Outline,
                          FLOAT32 XCenter,
                          FLOAT32 YCenter,
                          FLOAT32 XScale,
                          FLOAT32 YScale) {
/*
 **	Parameters:
 **		Outline			outline to be character normalized
 **		XCenter, YCenter	center point for normalization
 **		XScale, YScale		scale factors for normalization
 **	Globals: none
 **	Operation: This routine normalizes each point in Outline by
 **		translating it to the specified center and scaling it
 **		anisotropically according to the given scale factors.
 **	Return: none
 **	Exceptions: none
 **	History: Fri Dec 14 10:27:11 1990, DSJ, Created.
 */
  MFOUTLINE First, Current;
  MFEDGEPT *CurrentPoint;

  if (Outline == NIL)
    return;

  First = Outline;
  Current = First;
  do {
    CurrentPoint = PointAt (Current);
    XPositionOf (CurrentPoint) =
      (XPositionOf (CurrentPoint) - XCenter) * XScale;
    YPositionOf (CurrentPoint) =
      (YPositionOf (CurrentPoint) - YCenter) * YScale;

    Current = NextPointAfter (Current);
  }
  while (Current != First);

}                                /* CharNormalizeOutline */


/*---------------------------------------------------------------------------*/
void ComputeDirection(MFEDGEPT *Start,
                      MFEDGEPT *Finish,
                      FLOAT32 MinSlope,
                      FLOAT32 MaxSlope) {
/*
 **	Parameters:
 **		Start		starting point to compute direction from
 **		Finish		finishing point to compute direction to
 **		MinSlope	slope below which lines are horizontal
 **		MaxSlope	slope above which lines are vertical
 **	Globals: none
 **	Operation:
 **		This routine computes the slope from Start to Finish and
 **		and then computes the approximate direction of the line
 **		segment from Start to Finish.  The direction is quantized
 **		into 8 buckets:
 **			N, S, E, W, NE, NW, SE, SW
 **		Both the slope and the direction are then stored into
 **		the appropriate fields of the Start edge point.  The
 **		direction is also stored into the PreviousDirection field
 **		of the Finish edge point.
 **	Return: none
 **	Exceptions: none
 **	History: 7/25/89, DSJ, Created.
 */
  FVECTOR Delta;

  Delta.x = Finish->Point.x - Start->Point.x;
  Delta.y = Finish->Point.y - Start->Point.y;
  if (Delta.x == 0)
  if (Delta.y < 0) {
    Start->Slope = -MAX_FLOAT32;
    Start->Direction = south;
  }
  else {
    Start->Slope = MAX_FLOAT32;
    Start->Direction = north;
  }
  else {
    Start->Slope = Delta.y / Delta.x;
    if (Delta.x > 0)
      if (Delta.y > 0)
        if (Start->Slope > MinSlope)
          if (Start->Slope < MaxSlope)
            Start->Direction = northeast;
    else
      Start->Direction = north;
    else
      Start->Direction = east;
    else if (Start->Slope < -MinSlope)
    if (Start->Slope > -MaxSlope)
      Start->Direction = southeast;
    else
      Start->Direction = south;
    else
      Start->Direction = east;
    else if (Delta.y > 0)
    if (Start->Slope < -MinSlope)
      if (Start->Slope > -MaxSlope)
        Start->Direction = northwest;
    else
      Start->Direction = north;
    else
      Start->Direction = west;
    else if (Start->Slope > MinSlope)
    if (Start->Slope < MaxSlope)
      Start->Direction = southwest;
    else
      Start->Direction = south;
    else
      Start->Direction = west;
  }
  Finish->PreviousDirection = Start->Direction;
}                                /* ComputeDirection */


/*---------------------------------------------------------------------------*/
void FinishOutlineStats(register OUTLINE_STATS *OutlineStats) {
/*
 **	Parameters:
 **		OutlineStats	statistics about a set of outlines
 **	Globals: none
 **	Operation: Use the preliminary statistics accumulated in OutlineStats
 **		to compute the final statistics.
 **		(see Dan Johnson's Tesseract lab
 **		notebook #2, pgs. 74-78).
 **	Return: none
 **	Exceptions: none
 **	History: Fri Dec 14 10:13:36 1990, DSJ, Created.
 */
  OutlineStats->x = 0.5 * OutlineStats->My / OutlineStats->L;
  OutlineStats->y = 0.5 * OutlineStats->Mx / OutlineStats->L;

  OutlineStats->Ix = (OutlineStats->Ix / 3.0 -
    OutlineStats->y * OutlineStats->Mx +
    OutlineStats->y * OutlineStats->y * OutlineStats->L);

  OutlineStats->Iy = (OutlineStats->Iy / 3.0 -
    OutlineStats->x * OutlineStats->My +
    OutlineStats->x * OutlineStats->x * OutlineStats->L);

  /* Ix and/or Iy could possibly be negative due to roundoff error */
  if (OutlineStats->Ix < 0.0)
    OutlineStats->Ix = MIN_INERTIA;
  if (OutlineStats->Iy < 0.0)
    OutlineStats->Iy = MIN_INERTIA;

  OutlineStats->Rx = sqrt (OutlineStats->Ix / OutlineStats->L);
  OutlineStats->Ry = sqrt (OutlineStats->Iy / OutlineStats->L);

  OutlineStats->Mx *= 0.5;
  OutlineStats->My *= 0.5;

}                                /* FinishOutlineStats */


/*---------------------------------------------------------------------------*/
void InitOutlineStats(OUTLINE_STATS *OutlineStats) {
/*
 **	Parameters:
 **		OutlineStats	stats data structure to be initialized
 **	Globals: none
 **	Operation: Initialize the outline statistics data structure so
 **		that it is ready to start accumulating statistics.
 **	Return: none
 **	Exceptions: none
 **	History: Fri Dec 14 08:55:22 1990, DSJ, Created.
 */
  OutlineStats->Mx = 0.0;
  OutlineStats->My = 0.0;
  OutlineStats->L = 0.0;
  OutlineStats->x = 0.0;
  OutlineStats->y = 0.0;
  OutlineStats->Ix = 0.0;
  OutlineStats->Iy = 0.0;
  OutlineStats->Rx = 0.0;
  OutlineStats->Ry = 0.0;
}                                /* InitOutlineStats */


/*---------------------------------------------------------------------------*/
MFOUTLINE NextDirectionChange(MFOUTLINE EdgePoint) {
/*
 **	Parameters:
 **		EdgePoint	start search from this point
 **	Globals: none
 **	Operation:
 **		This routine returns the next point in the micro-feature
 **		outline that has a direction different than EdgePoint.  The
 **		routine assumes that the outline being searched is not a
 **		degenerate outline (i.e. it must have 2 or more edge points).
 **	Return: Point of next direction change in micro-feature outline.
 **	Exceptions: none
 **	History: 7/25/89, DSJ, Created.
 */
  DIRECTION InitialDirection;

  InitialDirection = DirectionOf (PointAt (EdgePoint));

  do
  EdgePoint = NextPointAfter (EdgePoint);
  while (DirectionOf (PointAt (EdgePoint)) == InitialDirection);

  return (EdgePoint);
}                                /* NextDirectionChange */


/*---------------------------------------------------------------------------*/
void UpdateOutlineStats(register OUTLINE_STATS *OutlineStats,
                        register FLOAT32 x1,
                        register FLOAT32 x2,
                        register FLOAT32 y1,
                        register FLOAT32 y2) {
/*
 **	Parameters:
 **		OutlineStats	statistics to add this segment to
 **		x1, y1, x2, y2	segment to be added to statistics
 **	Globals: none
 **	Operation: This routine adds the statistics for the specified
 **		line segment to OutlineStats.  The statistics that are
 **		kept are:
 **			sum of length of all segments
 **			sum of 2*Mx for all segments
 **			sum of 2*My for all segments
 **			sum of 2*Mx*(y1+y2) - L*y1*y2 for all segments
 **			sum of 2*My*(x1+x2) - L*x1*x2 for all segments
 **		These numbers, once collected can later be used to easily
 **		compute the center of mass, first and second moments,
 **		and radii of gyration.  (see Dan Johnson's Tesseract lab
 **		notebook #2, pgs. 74-78).
 **	Return: none
 **	Exceptions: none
 **	History: Fri Dec 14 08:59:17 1990, DSJ, Created.
 */
  register FLOAT64 L;
  register FLOAT64 Mx2;
  register FLOAT64 My2;

  /* compute length of segment */
  L = sqrt ((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
  OutlineStats->L += L;

  /* compute 2Mx and 2My components */
  Mx2 = L * (y1 + y2);
  My2 = L * (x1 + x2);
  OutlineStats->Mx += Mx2;
  OutlineStats->My += My2;

  /* compute second moment component */
  OutlineStats->Ix += Mx2 * (y1 + y2) - L * y1 * y2;
  OutlineStats->Iy += My2 * (x1 + x2) - L * x1 * x2;

}                                /* UpdateOutlineStats */
