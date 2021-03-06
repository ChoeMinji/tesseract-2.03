/******************************************************************************
 **	Filename:    featdefs.c
 **	Purpose:     Definitions of currently defined feature types.
 **	Author:      Dan Johnson
 **	History:     Mon May 21 10:26:21 1990, DSJ, Created.
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
#include "featdefs.h"
#include "emalloc.h"
#include "danerror.h"
#include "scanutils.h"
#include "variables.h"
#include "sigmenu.h"

#include <string.h>
#include <stdio.h>

/* define errors triggered by this module */
#define ILLEGAL_NUM_SETS  3001

#define PICO_FEATURE_LENGTH 0.05
#define MAX_OUTLINE_FEATURES  100

/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
/* define all of the parameters for the MicroFeature type*/
StartParamDesc (MicroFeatureParams)
DefineParam (0, 0, -0.5, 0.5)
DefineParam (0, 0, -0.25, 0.75)
DefineParam (0, 1, 0.0, 1.0)
DefineParam (1, 0, 0.0, 1.0)
DefineParam (0, 1, -0.5, 0.5)
DefineParam (0, 1, -0.5, 0.5)
EndParamDesc
/* now define the feature type itself (see features.h for info about each
  parameter).*/
DefineFeature (MicroFeatureDesc, 5, 1, 4, 50, "Micro", "mf", MicroFeatureParams)

// define all of the parameters for the PicoFeature type
/* define knob that can be used to adjust pico-feature length */
FLOAT32 PicoFeatureLength = PICO_FEATURE_LENGTH;
StartParamDesc (PicoFeatParams)
DefineParam (0, 0, -0.25, 0.75)
DefineParam (1, 0, 0.0, 1.0)
DefineParam (0, 0, -0.5, 0.5)
EndParamDesc
/* now define the feature type itself (see features.h for info about each
  parameter).*/
DefineFeature (PicoFeatDesc, 2, 1, 1, MAX_UINT8, "Pico", "pf", PicoFeatParams)

/* define all of the parameters for the NormFeat type*/
StartParamDesc (CharNormParams)
DefineParam (0, 0, -0.25, 0.75)
DefineParam (0, 1, 0.0, 1.0)
DefineParam (0, 1, 0.0, 1.0)
DefineParam (0, 1, 0.0, 1.0)
EndParamDesc
/* now define the feature type itself (see features.h for info about each
  parameter).*/
DefineFeature (CharNormDesc, 4, 0, 1, 1, "CharNorm", "cn", CharNormParams)

// define all of the parameters for the OutlineFeature type
StartParamDesc (OutlineFeatParams)
DefineParam (0, 0, -0.5, 0.5)
DefineParam (0, 0, -0.25, 0.75)
DefineParam (0, 0, 0.0, 1.0)
DefineParam (1, 0, 0.0, 1.0)
EndParamDesc
/* now define the feature type itself (see features.h for info about each
  parameter).*/
DefineFeature (OutlineFeatDesc, 3, 1, 1, MAX_OUTLINE_FEATURES, "Outline",
               "of", OutlineFeatParams)

/**----------------------------------------------------------------------------
        Global Data Definitions and Declarations
----------------------------------------------------------------------------**/
FEATURE_DEFS_STRUCT FeatureDefs = {
  NUM_FEATURE_TYPES,
  {
    &MicroFeatureDesc,
      &PicoFeatDesc,
      &OutlineFeatDesc,
      &CharNormDesc
  }
};

/**----------------------------------------------------------------------------
              Public Code
----------------------------------------------------------------------------**/
/*---------------------------------------------------------------------------*/
void FreeCharDescription(CHAR_DESC CharDesc) {
/*
 **	Parameters:
 **		CharDesc	character description to be deallocated
 **	Globals: none
 **	Operation: Release the memory consumed by the specified character
 **		description and all of the features in that description.
 **	Return: none
 **	Exceptions: none
 **	History: Wed May 23 13:52:19 1990, DSJ, Created.
 */
  int i;

  if (CharDesc) {
    for (i = 0; i < NumFeatureSetsIn (CharDesc); i++)
      FreeFeatureSet (FeaturesOfType (CharDesc, i));
    Efree(CharDesc);
  }
}                                /* FreeCharDescription */


/*---------------------------------------------------------------------------*/
CHAR_DESC NewCharDescription() {
/*
 **	Parameters: none
 **	Globals: none
 **	Operation: Allocate a new character description, initialize its
 **		feature sets to be empty, and return it.
 **	Return: New character description structure.
 **	Exceptions: none
 **	History: Wed May 23 15:27:10 1990, DSJ, Created.
 */
  CHAR_DESC CharDesc;
  int i;

  CharDesc = (CHAR_DESC) Emalloc (sizeof (CHAR_DESC_STRUCT));
  NumFeatureSetsIn (CharDesc) = NumFeaturesDefined ();

  for (i = 0; i < NumFeatureSetsIn (CharDesc); i++)
    FeaturesOfType (CharDesc, i) = NULL;

  return (CharDesc);

}                                /* NewCharDescription */


/*---------------------------------------------------------------------------*/
void WriteCharDescription(FILE *File, CHAR_DESC CharDesc) {
/*
 **	Parameters:
 **		File		open text file to write CharDesc to
 **		CharDesc	character description to write to File
 **	Globals: none
 **	Operation: Write a textual representation of CharDesc to File.
 **		The format used is to write out the number of feature
 **		sets which will be written followed by a representation of
 **		each feature set.
 **		Each set starts with the short name for that feature followed
 **		by a description of the feature set.  Feature sets which are
 **		not present are not written.
 **	Return: none
 **	Exceptions: none
 **	History: Wed May 23 17:21:18 1990, DSJ, Created.
 */
  int Type;
  int NumSetsToWrite = 0;

  for (Type = 0; Type < NumFeatureSetsIn (CharDesc); Type++)
    if (FeaturesOfType (CharDesc, Type))
      NumSetsToWrite++;

  fprintf (File, " %d\n", NumSetsToWrite);
  for (Type = 0; Type < NumFeatureSetsIn (CharDesc); Type++)
  if (FeaturesOfType (CharDesc, Type)) {
    fprintf (File, "%s ", ShortNameOf (DefinitionOf (Type)));
    WriteFeatureSet (File, FeaturesOfType (CharDesc, Type));
  }
}                                /* WriteCharDescription */


/*---------------------------------------------------------------------------*/
CHAR_DESC ReadCharDescription(FILE *File) {
/*
 **	Parameters:
 **		File	open text file to read character description from
 **	Globals: none
 **	Operation: Read a character description from File, and return
 **		a data structure containing this information.  The data
 **		is formatted as follows:
 **			NumberOfSets
 **				ShortNameForSet1 Set1
 **				ShortNameForSet2 Set2
 **				...
 **	Return: Character description read from File.
 **	Exceptions: ILLEGAL_NUM_SETS
 **	History: Wed May 23 17:32:48 1990, DSJ, Created.
 */
  int NumSetsToRead;
  char ShortName[FEAT_NAME_SIZE];
  CHAR_DESC CharDesc;
  int Type;

  if (fscanf (File, "%d", &NumSetsToRead) != 1 ||
    NumSetsToRead < 0 || NumSetsToRead > NumFeaturesDefined ())
    DoError (ILLEGAL_NUM_SETS, "Illegal number of feature sets");

  CharDesc = NewCharDescription ();
  for (; NumSetsToRead > 0; NumSetsToRead--) {
    fscanf (File, "%s", ShortName);
    Type = ShortNameToFeatureType (ShortName);
    FeaturesOfType (CharDesc, Type) =
      ReadFeatureSet (File, DefinitionOf (Type));
  }
  return (CharDesc);

}                                // ReadCharDescription


/*---------------------------------------------------------------------------*/
int ShortNameToFeatureType(const char *ShortName) {
/*
 **	Parameters:
 **		ShortName	short name of a feature type
 **	Globals: none
 **	Operation: Search thru all features currently defined and return
 **		the feature type for the feature with the specified short
 **		name.  Trap an error if the specified name is not found.
 **	Return: Feature type which corresponds to ShortName.
 **	Exceptions: ILLEGAL_SHORT_NAME
 **	History: Wed May 23 15:36:05 1990, DSJ, Created.
 */
  int i;

  for (i = 0; i < NumFeaturesDefined (); i++)
    if (!strcmp (ShortNameOf (DefinitionOf (i)), ShortName))
      return (i);
  DoError (ILLEGAL_SHORT_NAME, "Illegal short name for a feature");
  return 0;

}                                // ShortNameToFeatureType
