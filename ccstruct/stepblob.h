/**********************************************************************
 * File:        stepblob.h  (Formerly cblob.h)
 * Description: Code for C_BLOB class.
 * Author:		Ray Smith
 * Created:		Tue Oct 08 10:41:13 BST 1991
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#ifndef           STEPBLOB_H
#define           STEPBLOB_H

#include          "coutln.h"
#include          "rect.h"

class C_BLOB:public ELIST_LINK
{
  public:
    C_BLOB() {
    }                            //empty constructor
    C_BLOB(  //constructor //in random order
           C_OUTLINE_LIST *outline_list);

    C_OUTLINE_LIST *out_list() {  //get outline list
      return &outlines;
    }

    TBOX bounding_box();  //compute bounding box
    inT32 area();  //compute area
    inT32 outer_area();  //compute area
    inT32 count_transitions(                   //count maxima
                            inT32 threshold);  //size threshold

    void move(                    // reposition blob
              const ICOORD vec);  // by vector

    void plot(                       //draw one
              ScrollView* window,         //window to draw in
              ScrollView::Color blob_colour,    //for outer bits
              ScrollView::Color child_colour);  //for holes

    void prep_serialise() {  //set ptrs to counts
      outlines.prep_serialise ();
    }

    void dump(  //write external bits
              FILE *f) {
      outlines.dump (f);
    }

    void de_dump(  //read external bits
                 FILE *f) {
      outlines.de_dump (f);
    }

                                 //assignment
    make_serialise (C_BLOB) C_BLOB & operator= (
    const C_BLOB & source) {     //from this
      if (!outlines.empty ())
        outlines.clear ();

      outlines.deep_copy (&source.outlines);
      return *this;
    }

  private:
    C_OUTLINE_LIST outlines;     //master elements
};

ELISTIZEH_S (C_BLOB)
#endif
