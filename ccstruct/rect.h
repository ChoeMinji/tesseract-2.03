/**********************************************************************
 * File:        rect.h  (Formerly box.h)
 * Description: Bounding box class definition.
 * Author:					Phil Cheatle
 * Created:					Wed Oct 16 15:18:45 BST 1991
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

#ifndef           RECT_H
#define           RECT_H

#include          <math.h>
#include          "points.h"
#include          "ndminx.h"
#include          "tprintf.h"
#include	"scrollview.h"

class DLLSYM TBOX                 //bounding box
{
  public:
    TBOX ():                      //empty constructor
    bot_left (MAX_INT16, MAX_INT16), top_right (-MAX_INT16, -MAX_INT16) {
    }                            //null box

    TBOX(                    //constructor
        const ICOORD pt1,   //one corner
        const ICOORD pt2);  //the other corner
    TBOX(  //box around FCOORD
        const FCOORD pt);

    BOOL8 null_box() const {  //Is box null
      return ((left () > right ()) || (top () < bottom ()));
    }

    inT16 top() const {  // coord of top
      return top_right.y ();
    }

    inT16 bottom() const {  // coord of bottom
      return bot_left.y ();
    }

    inT16 left() const {  // coord of left
      return bot_left.x ();
    }

    inT16 right() const {  // coord of right
      return top_right.x ();
    }

                                 //access function
    const ICOORD &botleft() const {
      return bot_left;
    }

    ICOORD botright() const {  // ~ access function
      return ICOORD (top_right.x (), bot_left.y ());
    }

    ICOORD topleft() const {  // ~ access function
      return ICOORD (bot_left.x (), top_right.y ());
    }

                                 //access function
    const ICOORD &topright() const {
      return top_right;
    }

    inT16 height() const {  //how high is it?
      if (!null_box ())
        return top_right.y () - bot_left.y ();
      else
        return 0;
    }

    inT16 width() const {  //how high is it?
      if (!null_box ())
        return top_right.x () - bot_left.x ();
      else
        return 0;
    }

    inT32 area() const {  //what is the area?
      if (!null_box ())
        return width () * height ();
      else
        return 0;
    }

    void move_bottom_edge(                  // move one edge
                          const inT16 y) {  // by +/- y
      bot_left += ICOORD (0, y);
    }

    void move_left_edge(                  // move one edge
                        const inT16 x) {  // by +/- x
      bot_left += ICOORD (x, 0);
    }

    void move_right_edge(                  // move one edge
                         const inT16 x) {  // by +/- x
      top_right += ICOORD (x, 0);
    }

    void move_top_edge(                  // move one edge
                       const inT16 y) {  // by +/- y
      top_right += ICOORD (0, y);
    }

    void move(                     // move box
              const ICOORD vec) {  // by vector
      bot_left += vec;
      top_right += vec;
    }

    void move(                     // move box
              const FCOORD vec) {  // by float vector
      bot_left.set_x ((inT16) floor (bot_left.x () + vec.x ()));
      //round left
      bot_left.set_y ((inT16) floor (bot_left.y () + vec.y ()));
      //round down

      top_right.set_x ((inT16) ceil (top_right.x () + vec.x ()));
      //round right
      top_right.set_y ((inT16) ceil (top_right.y () + vec.y ()));
      //round up
    }

    void scale(                  // scale box
               const float f) {  // by multiplier
                                 //round left
      bot_left.set_x ((inT16) floor (bot_left.x () * f));
                                 //round down
      bot_left.set_y ((inT16) floor (bot_left.y () * f));

      top_right.set_x ((inT16) ceil (top_right.x () * f));
      //round right
      top_right.set_y ((inT16) ceil (top_right.y () * f));
      //round up
    }
    void scale(                     // scale box
               const FCOORD vec) {  // by float vector
      bot_left.set_x ((inT16) floor (bot_left.x () * vec.x ()));
      bot_left.set_y ((inT16) floor (bot_left.y () * vec.y ()));
      top_right.set_x ((inT16) ceil (top_right.x () * vec.x ()));
      top_right.set_y ((inT16) ceil (top_right.y () * vec.y ()));
    }

    void rotate(                     //rotate coords
                const FCOORD vec) {  //by vector
      bot_left.rotate (vec);
      top_right.rotate (vec);
      *this = TBOX (bot_left, top_right);
    }

    BOOL8 contains(  //is pt inside box
                   const FCOORD pt) const;

    BOOL8 contains(  //is box inside box
                   const TBOX &box) const;

    BOOL8 overlap(  //do boxes overlap
                  const TBOX &box) const;

    BOOL8 major_overlap(  // Do boxes overlap more than half.
                        const TBOX &box) const;

    TBOX intersection(  //shared area box
                     const TBOX &box) const;

    TBOX bounding_union(  //box enclosing both
                       const TBOX &box) const;

    void print() {  //print
      tprintf ("Bounding box=(%d,%d)->(%d,%d)\n",
        left (), bottom (), right (), top ());
    }

#ifndef GRAPHICS_DISABLED
    void plot(                    //use current settings
              ScrollView* fd) const {  //where to paint
      fd->Rectangle(bot_left.x (), bot_left.y (), top_right.x (),
        top_right.y ());
    }

    void plot(                              //paint box
              ScrollView* fd,                    //where to paint
              ScrollView::Color fill_colour,           //colour for inside
              ScrollView::Color border_colour) const;  //colour for border
#endif

    friend DLLSYM TBOX & operator+= (TBOX &, const TBOX &);
    //in place union
    friend DLLSYM TBOX & operator-= (TBOX &, const TBOX &);
    //in place intrsection

    void serialise_asc(  //convert to ascii
                       FILE *f);
    void de_serialise_asc(  //convert from ascii
                          FILE *f);

  private:
    ICOORD bot_left;             //bottom left corner
    ICOORD top_right;            //top right corner
};

/**********************************************************************
 * TBOX::TBOX()  Constructor from 1 FCOORD
 *
 **********************************************************************/

inline TBOX::TBOX(                 //construtor
                const FCOORD pt  //floating centre
               ) {
  bot_left = ICOORD ((inT16) floor (pt.x ()), (inT16) floor (pt.y ()));
  top_right = ICOORD ((inT16) ceil (pt.x ()), (inT16) ceil (pt.y ()));
}


/**********************************************************************
 * TBOX::contains()  Is point within box
 *
 **********************************************************************/

inline BOOL8 TBOX::contains(const FCOORD pt) const {
  return ((pt.x () >= bot_left.x ()) &&
    (pt.x () <= top_right.x ()) &&
    (pt.y () >= bot_left.y ()) && (pt.y () <= top_right.y ()));
}


/**********************************************************************
 * TBOX::contains()  Is box within box
 *
 **********************************************************************/

inline BOOL8 TBOX::contains(const TBOX &box) const {
  return (contains (box.bot_left) && contains (box.top_right));
}


/**********************************************************************
 * TBOX::overlap()  Do two boxes overlap?
 *
 **********************************************************************/

inline BOOL8 TBOX::overlap(  //do boxes overlap
                          const TBOX &box) const {
  return ((box.bot_left.x () <= top_right.x ()) &&
    (box.top_right.x () >= bot_left.x ()) &&
    (box.bot_left.y () <= top_right.y ()) &&
    (box.top_right.y () >= bot_left.y ()));
}

/**********************************************************************
 * TBOX::major_overlap()  Do two boxes overlap by at least half of the smallest?
 *
 **********************************************************************/

inline BOOL8 TBOX::major_overlap(  // Do boxes overlap more that half.
                                const TBOX &box) const {
  int overlap = MIN(box.top_right.x(), top_right.x());
  overlap -= MAX(box.bot_left.x(), bot_left.x());
  overlap += overlap;
  if (overlap < MIN(box.width(), width()))
    return false;
  overlap = MIN(box.top_right.y(), top_right.y());
  overlap -= MAX(box.bot_left.y(), bot_left.y());
  overlap += overlap;
  if (overlap < MIN(box.height(), height()))
    return false;
  return true;
}
#endif
