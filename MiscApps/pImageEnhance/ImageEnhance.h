/*************************************************************************

    SGMOOS: A set of libraries, scripts & applications which extend MOOS 
    (Mission Oriented Operating Suite by Dr. Paul Newman) for use in 
    Autonomous Underwater Vehicles & Autonomous Surface Vessels. 

    Copyright (C) 2006,2007,2008,2009 Massachusetts Institute of 
    Technology Sea Grant

    This software was written by Ian Katz & others at MIT Sea Grant.
    contact: ijk5@mit.edu

    This file is part of SGMOOS.

    SGMOOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SGMOOS.  If not, see <http://www.gnu.org/licenses/>.

*************************************************************************/
//
//ImageEnhance.h - interface for the CImageEnhance class
//
/////////////////////////////////////


#if !defined(AFX_IMAGEENHANCE_H_INCLUDED_)
#define AFX_IMAGEENHANCE_H_INCLUDED_

#include <MOOSLIB/MOOSLib.h>

#include <Magick++.h>
#include <math.h>
#include <string.h>
#include <list>

using namespace std;
using namespace Magick;



//struct to hold information about a MOOSVAR (which we use to draw on the image)
typedef struct
{
    string Label;
    string MOOSVar;
    string value;
    string format;
    int length;
} SVal;


//representing a line of text on the image
class CLineItem
{
protected:
    string pad(string s, int l) {s.resize(l, ' '); return s;}

public:
    CLineItem() {};
    virtual ~CLineItem() {};

    virtual string text() = 0;

};


//class that holds a RAW STRING to go on the image
class CLineString : public CLineItem
{
protected:
    string m_sLine;

public:
    CLineString() {};

    void setText(string s) {m_sLine = s;}
    virtual string text() {return m_sLine;}
};


//class that prints a series of variables with friendly labels
class CLineVars : public CLineItem
{
protected:

    typedef list<SVal*> SVAL_LIST;

    SVAL_LIST m_Vals;

public:
    CLineVars() {};

    ~CLineVars()
    {
        for (SVAL_LIST::iterator it = m_Vals.begin(); it != m_Vals.end(); it++) 
            delete (*it);
    }

    void addVar(SVal* s) {m_Vals.push_back(s);}
    
    virtual string text() 
    {
        string ret = "";
        for (SVAL_LIST::iterator it = m_Vals.begin(); it != m_Vals.end(); it++)
            ret += pad((*it)->Label + ": " + (*it)->value, (*it)->length);
        return ret;
    }
    
};


//the actual app
class CImageEnhanceApp : public CMOOSApp
{
public:
    CImageEnhanceApp() {};
    virtual ~CImageEnhanceApp();
    
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool OnStartUp();
    bool Iterate();

protected:
    
    int     m_iFontSizePx;
    double  m_dfViewAngleX;
    double  m_dfViewAngleY;
    double  m_dfBarSize;
    string  m_sRangeVar;
    bool    m_bProcessNow;
    string  m_sFileToProcess;
    string  m_sThumbSize;

    double  m_dfStartTime;

    typedef list<CLineItem*> LINE_LIST;
    LINE_LIST m_Lines;

    typedef map<string, SVal*> VAR_MAP;
    VAR_MAP m_MOOSvarMap;

    bool hasFullInit()
    {
        return m_iFontSizePx != -1
            && m_dfViewAngleX != -1
            && m_dfViewAngleY != -1
            && m_dfBarSize != -1
            && !m_sThumbSize.empty()
            && !m_sRangeVar.empty();
    }

    string mkFont()
    {
        char s[10];
        sprintf(s, "%d", m_iFontSizePx);
        return string("-*-terminus-medium-r-*-*-") + s + "-*-*-*-*-*-*-*";
    }


    void str_replace(string &str, const string &find_what, const string &replace_with)
    {
        string::size_type pos = 0;
        
        while((pos=str.find(find_what, pos))!=string::npos)
        {
            str.erase(pos, find_what.length());
            str.insert(pos, replace_with);
            pos+=replace_with.length();
        }
    }

    bool BuildInfoArea();
    CLineString* parseScreentext(string txt);
    CLineVars* parseMOOStext(string txt);



public:

    void Process();

    void Enhance(Image &myImage);  //color-normalize, sharpen, etc
    Image Annotate(Image myImage); //add stuff to the image, like values
    void drawMITlogo(Image &myCanvas, int x, int y);


    //aCanvas is the target
    //iPix_x and iPix_y are the boundaries for drawing inside the canvas
    //iPos is where we start drawing
    //shade_w and shade_b are a channel (one of RGB) value
    static void drawVertBar(Image& aCanvas, int iPix_x, int iPix_y, int iPos, int shade_w, int shade_b);

    //aCanvas is the target
    //iPix_x and iPix_y are the boundaries for drawing inside the canvas
    //iPos is where we start drawing
    //shade_w and shade_b are a channel (one of RGB) value
    static void drawHorizBar(Image& aCanvas, int iPix_x, int iPix_y, int iPos, int shade_w, int shade_b);



    //iMin and iMax specify the range of iteration
    //iPix_x and iPix_y specify the starting point in the image
    //dfSinglepx_size is the physical width of the subject represented by a single pixel
    //dfBar_size is the length ???? forgot 
    //anImage... what we work on
    // the work function
    void iterateScaleBars(int iMin, int iMax, 
                        int iPix_x, int iPix_y,
                        double dfSinglepx_size, double dfBar_size, 
                        Image& anImage,
                        void (*work)(Image& im, 
                                     int iPix_x, 
                                     int iPix_y, 
                                     int iPos, 
                                     int shade_w, 
                                     int shade_b));

};


#endif //AFX_IMAGEENHANCE_H_INCLUDED_
