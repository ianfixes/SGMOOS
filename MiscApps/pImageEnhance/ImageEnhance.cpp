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

#include "ImageEnhance.h"

CImageEnhanceApp::~CImageEnhanceApp()
{
    //clear out dynamic stuff
    for (LINE_LIST::iterator it = m_Lines.begin(); it != m_Lines.end(); it++) 
        delete (*it);

}


/**
 * Initialization
 * 
 * Make sure all config params are available, print an error if not
 * load all member variables 
 */
bool CImageEnhanceApp::OnStartUp()
{
    std::string s;
    
    std::string sUsage = 
        "The pImageEnhance application expects the following configuration parameters:\n"
        "\tFontSize_px      = <font size in pixels (not all are supported... try 14)>\n"
        "\tViewAngle_rad_x  = <Viewing angle of camera (width), in radians>\n"
        "\tViewAngle_rad_y  = <Viewing angle of camera (height), in radians>\n"
        "\tBarSize_m        = <size of scale bars on image, in meters>\n"
        "\tRangeVar         = <MOOS variable indicating distance of camera from subject>\n"
        "\tThumbSize_px     = <width x height, for example '320x240'>\n"
//        "\t = <>\n"
        "\n";

    //init variables
    m_iFontSizePx = -1;
    m_dfViewAngleX = -1;
    m_dfViewAngleY = -1;
    m_dfBarSize = -1;
    m_sRangeVar = "";

    m_bProcessNow = false;

    //get various vars
    m_MissionReader.GetConfigurationParam("fontsize_px", s)      && (m_iFontSizePx  = atoi(s.c_str()));
    m_MissionReader.GetConfigurationParam("viewangle_rad_x", s)  && (m_dfViewAngleX = atof(s.c_str()));
    m_MissionReader.GetConfigurationParam("viewangle_rad_y", s)  && (m_dfViewAngleY = atof(s.c_str()));
    m_MissionReader.GetConfigurationParam("barsize_m", s)        && (m_dfBarSize    = atof(s.c_str()));

    m_MissionReader.GetConfigurationParam("rangeVar",     m_sRangeVar);
    m_MissionReader.GetConfigurationParam("thumbsize_px", m_sThumbSize);

    if (!hasFullInit())
    {
        MOOSTrace("\nMOOS MISSION FILE CONFIGURATION ERROR!\n");
        MOOSTrace(sUsage);
        MOOSTrace("\nThe Image Enhancement application cannot continue.\n");

        return false;
    }

    //create entry in table for the range variable
    m_MOOSvarMap[m_sRangeVar] = new SVal();
    m_MOOSvarMap[m_sRangeVar]->format = "%f";

    if (!BuildInfoArea())
    {
        MOOSTrace("Problem parsing/building info area");
        return false;
    }


    //subscribe to all vars
    for (VAR_MAP::iterator it = m_MOOSvarMap.begin(); it != m_MOOSvarMap.end(); it++) 
    {
        if (!m_Comms.Register(it->first, 0.2)) 
            return false;
    }

    return m_Comms.Register("ICAMERA_SAVEDFILE", 0.2);
}

bool CImageEnhanceApp::BuildInfoArea()
{
    
    //figure out what we are required to log....
    //here we read in what we want to log from the mission file..
    STRING_LIST Params;
    if (m_MissionReader.GetConfiguration(m_sAppName, Params))
    {
        //parse in the order that they were declared in *.moos file
        Params.reverse();
        
        STRING_LIST::iterator p;
        for (p = Params.begin(); p != Params.end(); p++)
        {
            std::string sParam = *p;
            std::string sWhat = MOOSChomp(sParam, "=");
            
            if (MOOSStrCmp(sWhat,      "LINE_PLAINTEXT"))
            {
                m_Lines.push_back(parseScreentext(sParam));
            }
            else if (MOOSStrCmp(sWhat, "LINE_VARSTRING"))
            {
                m_Lines.push_back(parseMOOStext(sParam));
            }
        }
    }
    
    return true;
    
}


//parse screentext line     
// screentext = stuff`that`gets`put`on`the`image`literally
CLineString* CImageEnhanceApp::parseScreentext(string txt)
{
    CLineString* ret = new CLineString();
    //mission file reader nukes our spaces, so use backticks
    str_replace(txt, "`", " ");
    ret->setText(txt);
    return ret;
}


//parse moostext line       
// moostext = NAV_ALTITUDE,Altitude,%2.3f,30;NAV_LAT,Latitude,%3.6f,20;A_STRING,A`string,%s,15
CLineVars* CImageEnhanceApp::parseMOOStext(string txt)
{
    CLineVars* ret = new CLineVars();
    SVal* pack;
    string phrase;

    //iterate through all segments declaring
    for (phrase = MOOSChomp(txt, ";"); !phrase.empty(); phrase = MOOSChomp(txt, ";"))
    {
        pack = new SVal();

        //varname
        pack->MOOSVar = MOOSChomp(phrase, ",");

        //friendly name
        pack->Label = MOOSChomp(phrase, ",");
        str_replace(pack->Label, "`", " "); //convert backticks to spaces
        
        //format
        pack->format = MOOSChomp(phrase, ",");

        //length
        pack->length = atoi(phrase.c_str());

        ret->addVar(pack);

        //add to our map so we can update it
        m_MOOSvarMap[pack->MOOSVar] = pack;
    }


    return ret;
}

bool CImageEnhanceApp::Iterate()
{
    if (m_bProcessNow)
    {
        Process();
        m_bProcessNow = false;
    }
    return true;
}


bool CImageEnhanceApp::OnNewMail(MOOSMSG_LIST &NewMail)
{
    VAR_MAP::iterator it;    
    CMOOSMsg Msg;
    char buf[100];

    //iterate through map of vars
    for (it = m_MOOSvarMap.begin(); it != m_MOOSvarMap.end(); it++)
    {
        //check for var in new mail
        if (m_Comms.PeekMail(NewMail, it->first, Msg))
        { 
            if (!Msg.IsSkewed(MOOSTime()))
            {
                //format new val properly as string
                //insert new val into struct
                if (Msg.IsString())
                {
                    it->second->value = Msg.GetString();
                }
                else
                {
                    sprintf(buf, it->second->format.c_str(), Msg.GetDouble());
                    it->second->value = buf;
                }
            }
        }
    }

    //if var is for a new image, set our flag
    if (m_Comms.PeekMail(NewMail, "ICAMERA_SAVEDFILE", Msg))
    {
        if (!Msg.IsSkewed(MOOSTime()))
        {
            m_sFileToProcess = Msg.GetString();
            MOOSTrace("\nGot new file to process: %s", m_sFileToProcess.c_str());
            m_bProcessNow = true;
        }
    }

    return true;
}


void CImageEnhanceApp::Enhance(Image &myImage)
{
    Image::ImageStatistics stats;

    //auto-level
    myImage.statistics(&stats);
    myImage.levelChannel(RedChannel, stats.red.minimum, stats.red.maximum);
    myImage.levelChannel(GreenChannel, stats.green.minimum, stats.green.maximum);
    myImage.levelChannel(BlueChannel, stats.blue.minimum, stats.blue.maximum);

    //unsharp mask
    myImage.unsharpmask(0, 1.6, 1, 0);

}

void CImageEnhanceApp::Process()
{
    //m_sFileToProcess = "/data/images/iCamera_2009-10-15_1226_session0004/000509.ppm"; //FIXME, deleteme
    
    m_dfStartTime = MOOSTime();

    //the varname for errors
    string var_error = GetAppName() + "_ERROR";
    MOOSToUpper(var_error);
    
    //separate the path and extension from the filename... save the path, we need it
    size_t lastslash = m_sFileToProcess.rfind("/");
    string basepath  = m_sFileToProcess.substr(0, lastslash + 1);
    string imgfile   = m_sFileToProcess.substr(lastslash + 1);
    imgfile = imgfile.substr(0, imgfile.find("."));


    //create a directory for processed images if it doesn't already exist
    string processed = basepath + "processed";
    try
    {
        mkdir(processed.c_str(), 0777);
    }
    catch (...)
    {
        m_Comms.Notify(var_error, (string)"Couldn't create directory for saved images: " + processed);
        return;
    }

    //create filenames for the new images we're making
    string newfile_base = processed + "/" + imgfile;
    string newfile_jpg  = newfile_base + ".jpg";
    string newfile_thm  = newfile_base + ".thumb.jpg";
    string newfile_bmp  = newfile_base + ".tiff";

    //enhance... enhance... enhance...
    try
    {
        //now create the image and do all the fun stuff
        Image myImage(m_sFileToProcess);

        Enhance(myImage);
        Image myThumb = myImage;
        myThumb.scale(m_sThumbSize);
        myThumb.write(newfile_thm);
        
        myImage = Annotate(myImage);
        
        //myImage.display();

        //myImage.quality(95); //for PNG, THIS MEANS zlib compression = 9 and filtering = 5 (adaptive)
        myImage.depth(8); //for TIFF
        myImage.write(newfile_bmp);

        myImage.quality(85); //this is a straight-up percentage
        myImage.write(newfile_jpg);

        //notify the DB of the new files we made
        string v = GetAppName() + "_OUTPUT";
        MOOSToUpper(v);

        string output = m_sFileToProcess + ":"
            + "BITMAP=" + newfile_bmp 
            + ";JPEG=" + newfile_jpg 
            + ";THUMB=" + newfile_thm + ";";

        m_Comms.Notify(v, output);
    }
    catch (...)
    {
        m_Comms.Notify(var_error, (string)"Couldn't load, enhance or save images (tried "
                       + m_sFileToProcess + ")... you figure out what went wrong");
        return;
    }

    MOOSTrace("\nProcessing took %f seconds", MOOSTime() - m_dfStartTime);

}


void CImageEnhanceApp::drawMITlogo(Image &myCanvas, int x, int y)
{
    //double halftone = MaxRGB / 2;
    Color black = Color(0, 0, 0);
    Color MIT_red = Color(MaxRGB * 0.6, MaxRGB * 0.2, MaxRGB * 0.2);
    Color MIT_grey = Color(MaxRGB * 0.4, MaxRGB * 0.4, MaxRGB * 0.4);

    myCanvas.strokeWidth(0.0);
    myCanvas.fillColor(black);
    myCanvas.strokeColor(black);
    myCanvas.draw(DrawableRectangle(x, y, x + 29, y + 18));

    myCanvas.fillColor(MIT_red);
    myCanvas.strokeColor(MIT_red);
    myCanvas.draw(DrawableRectangle(x + 2, y + 3, x + 4, y + 16));
    myCanvas.draw(DrawableRectangle(x + 7, y + 3, x + 9, y + 12));
    myCanvas.draw(DrawableRectangle(x + 12, y + 3, x + 14, y + 16));
    myCanvas.draw(DrawableRectangle(x + 17, y + 8, x + 19, y + 16));
    myCanvas.draw(DrawableRectangle(x + 22, y + 3, x + 28, y + 5));
    myCanvas.draw(DrawableRectangle(x + 22, y + 8, x + 24, y + 16));

    myCanvas.fillColor(MIT_grey);
    myCanvas.strokeColor(MIT_grey);
    myCanvas.draw(DrawableRectangle(x + 17, y + 3, x + 19, y + 5));

}


//draw scale bars and text on the image (or rather, return a new image with text and bars on it)
Image CImageEnhanceApp::Annotate(Image myImage)
{
    //create a relatively sized canvas
    int pix_x = myImage.columns();
    int pix_y = myImage.rows();

    //pix_y + 20 + (m_iFontSizePx * number of lines)
    Image myCanvas(Geometry(pix_x + 20, pix_y + 20 + m_iFontSizePx * m_Lines.size()), Color(0, 0, 0, 0));

    //draw scaling
    double m_ratio_x = 2 * tan(m_dfViewAngleX / 2);
    double m_ratio_y = 2 * tan(m_dfViewAngleY / 2);

    double dfCameraDist = atof(m_MOOSvarMap[m_sRangeVar]->value.c_str());
    double size_x = m_ratio_x * dfCameraDist;
    double size_y = m_ratio_y * dfCameraDist;

    double singlepx_x = size_x / pix_x;
    double singlepx_y = size_y / pix_y;

    double halftone = MaxRGB / 2;

    iterateScaleBars(0, pix_y - 1, pix_x, pix_y, singlepx_y, m_dfBarSize, myCanvas, &CImageEnhanceApp::drawVertBar);
    iterateScaleBars(0, pix_x - 1, pix_x, pix_y, singlepx_x, m_dfBarSize, myCanvas, &CImageEnhanceApp::drawHorizBar);

    //draw the box in the corner
    myCanvas.strokeWidth(0.0);
    myCanvas.fillColor(Color(halftone, halftone, halftone));
    myCanvas.draw(DrawableRectangle(pix_x + 1, pix_y + 1, pix_x + 19, pix_y + 19));

    //the logo
    drawMITlogo(myCanvas, pix_x + 1 + (18 - 29), pix_y + 1);

    //add text: CTD, altitude, date/time, lat/lon, physical size to nearest cm
    myCanvas.font(mkFont().c_str());
    myCanvas.fillColor(Color(MaxRGB, MaxRGB, MaxRGB));

    //iterate through m_Lines
    int base = pix_y + 20;
    int linenum = 1;
    for (LINE_LIST::iterator it = m_Lines.begin(); it != m_Lines.end(); it++)
    {
        myCanvas.draw(DrawableText(1, base + 14 * linenum++, (*it)->text()));
    }


    //composite
    myCanvas.composite(myImage, 1, 1, OverCompositeOp);

    return myCanvas;
}



//aCanvas is the target//iPix_x and iPix_y are the boundaries for drawing inside the canvas
//iPos is where we start drawing
//shade_w and shade_b are a channel (one of RGB) value
void CImageEnhanceApp::drawVertBar(Image& aCanvas, int iPix_x, int iPix_y, int iPos, int shade_w, int shade_b)
{
    aCanvas.strokeAntiAlias(false);
    aCanvas.strokeWidth(1.0);
    aCanvas.fillColor("red"); //because if we see red, we know we made a mistake
    int rownum = iPix_y - iPos;
    
    list<Coordinate> line_w;
    line_w.push_back(Coordinate(iPix_x + 2, rownum));
    line_w.push_back(Coordinate(iPix_x + 10, rownum));
    aCanvas.strokeColor(Color(shade_w, shade_w, shade_w, 0));
    aCanvas.draw(DrawablePolyline(line_w));
    
    list<Coordinate> line_b;
    line_b.push_back(Coordinate(iPix_x + 11, rownum));
    line_b.push_back(Coordinate(iPix_x + 18, rownum));
    aCanvas.strokeColor(Color(shade_b, shade_b, shade_b, 0));
    aCanvas.draw(DrawablePolyline(line_b));
}

//aCanvas is the target
//iPix_x and iPix_y are the boundaries for drawing inside the canvas
//iPos is where we start drawing
//shade_w and shade_b are a channel (one of RGB) value
void CImageEnhanceApp::drawHorizBar(Image& aCanvas, int iPix_x, int iPix_y, int iPos, int shade_w, int shade_b)
{
    aCanvas.strokeAntiAlias(false);
    aCanvas.strokeWidth(1.0);
    aCanvas.fillColor("red"); //because if we see red, we know we made a mistake
    int colnum = iPix_x - iPos;
    
    list<Coordinate> line_b;
    line_b.push_back(Coordinate(colnum, iPix_y + 2));
    line_b.push_back(Coordinate(colnum, iPix_y + 10));
    aCanvas.strokeColor(Color(shade_b, shade_b, shade_b, 0));
    aCanvas.draw(DrawablePolyline(line_b));
    
    list<Coordinate> line_w;
    line_w.push_back(Coordinate(colnum, iPix_y + 11));
    line_w.push_back(Coordinate(colnum, iPix_y + 18));
    aCanvas.strokeColor(Color(shade_w, shade_w, shade_w, 0));
    aCanvas.draw(DrawablePolyline(line_w));
}




void CImageEnhanceApp::iterateScaleBars(int iMin, int iMax, 
                                        int iPix_x, int iPix_y,
                                        double dfSinglepx_size, double dfBar_size, 
                                        Image& anImage,
                                        void (*work)(Image& im, 
                                                     int iPix_x, 
                                                     int iPix_y, 
                                                     int iPos, 
                                                     int shade_w, 
                                                     int shade_b))
{
    bool is_flip = false;
    bool is_white = true;
    double last_dist = 0;
    double new_dist;
    int shade_w;
    int shade_b;
    int shade_g;
    for (int i = iMin; i <= iMax; ++i)
    {
        new_dist = fmod((i + 1) * dfSinglepx_size, dfBar_size);
        is_flip = new_dist < last_dist;
        
        if (!is_flip)
        {
            shade_w = is_white ? MaxRGB : 0;
            shade_b = is_white ? 0 : MaxRGB;
        }
        else
        {
            shade_g = round(MaxRGB * new_dist / (dfBar_size * dfSinglepx_size));

            if (is_white)
            {
                shade_w = shade_g;
                shade_b = MaxRGB - shade_g;
            }
            else
            {
                shade_b = shade_g;
                shade_w = MaxRGB - shade_g;
            }
            is_white = !is_white;
        }

        last_dist = new_dist;

        //now do work
        (work)(anImage, iPix_x, iPix_y, i, shade_w, shade_b);

    }

}

