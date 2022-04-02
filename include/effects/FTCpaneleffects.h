//+--------------------------------------------------------------------------
//
// File:        FTCpaneleffects.h
//
// Description:
//
//   Display team number, alliance and status items on FTC robot 
//   by Team 6832, Iron Reign
//
// History:     Mar-16-2022         
//
//---------------------------------------------------------------------------

#pragma once

#include <sys/types.h>
#include <errno.h>
#include <iostream>
#include <vector>
#include <math.h>
#include <deque>
#include "colorutils.h"
#include "globals.h"
#include "ledstripeffect.h"
#include "particles.h"
#include "screen.h"
#include "ironfont.h"


extern AppTime  g_AppTime;
//extern PeakData g_Peaks;
extern DRAM_ATTR uint8_t giInfoPage;                   // Which page of the display is being shown
extern DRAM_ATTR bool gbInfoPageDirty;
#define MS_PER_SECOND 1000
#define gVURatio 2
#define NUM_CHARS 5

// Hacked from Dave's SpectrumAnalyzerEffect
//
// An effect that draws an FTC robot dispaly on two 8x32 panels
// Todo - add the second panel

class FTCPanelEffect : public LEDStripEffect
{
  protected:

    uint8_t   _colorOffset;
    uint16_t  _scrollSpeed;
    uint8_t   _fadeRate;

    int panelWidth=32;
    int panelStart = 0;

    bool allianceIsRed = false;

    CRGBPalette256 _palette;

    enum Commands {
        CHANGE_LED_LENGTH=0x70,
        WRITE_SINGLE_LED_COLOR=0x71,
        WRITE_ALL_LED_COLOR=0x72,
        WRITE_RED_ARRAY=0x73,
        WRITE_GREEN_ARRAY=0x74,
        WRITE_BLUE_ARRAY=0x75,
        WRITE_SINGLE_LED_BRIGHTNESS=0x76,
        WRITE_ALL_LED_BRIGHTNESS=0x77,
        WRITE_ALL_LED_OFF=0x78,
        WRITE_SET_ALLIANCE=0x79
    }; 


    //DrawAlliance
    //
    // Draw the current Alliance

    void DrawAlliance (bool allianceIsRed, int startX, int startY)
    {
        auto pGFXChannel = _GFX[0];
        if (allianceIsRed)
            //CRGB color = pGFXChannel->from16Bit(baseColor);
            pGFXChannel->fillRect(startX, startY, 6, 6, pGFXChannel->to16bit(CRGB(CRGB::Red)));
        else
        {
            pGFXChannel->fillRect(startX+1, startY+1, 4, 4, pGFXChannel->to16bit(CRGB(CRGB::Blue)));
            pGFXChannel->drawLine(startX+2,startY, startX+3, startY, pGFXChannel->to16bit(CRGB(CRGB::MediumBlue)));
            pGFXChannel->drawLine(startX+2,startY+5, startX+3, startY+5, pGFXChannel->to16bit(CRGB(CRGB::MediumBlue)));
            pGFXChannel->drawLine(startX,startY+2, startX, startY+3, pGFXChannel->to16bit(CRGB(CRGB::MediumBlue)));
            pGFXChannel->drawLine(startX+5,startY+2, startX+5, startY+3, pGFXChannel->to16bit(CRGB(CRGB::MediumBlue)));
            pGFXChannel->fillCircle(startX+3, startY+3, 3, pGFXChannel->to16bit(CRGB(CRGB::Blue)));
        }
    }

    // DrawChar
    //
    // Draws a given character from the IronFont

    void DrawChar(uint8_t iChar, uint16_t baseColor, char n, int panelStart)
    {
        auto pGFXChannel = _GFX[0];

        int charWidth = 4; 
        int xOffset   = iChar * (charWidth +1);
        int yOffset   = 0;
        int xStart = 8;
        
        int iRow = 0;
        CRGB color = pGFXChannel->from16Bit(baseColor);
        //pGFXChannel->fillCircle(3,3,3, CRGB(CRGB::BLUE16));
                
        for (int y = 0; y < 6; y++)
        {
            CRGB color = pGFXChannel->from16Bit(baseColor);
            iRow++;
            //pGFXChannel->drawLine(xOffset, y, xOffset+charWidth-1, y, pGFXChannel->to16bit(color));
            
            for (int x = 0; x < charWidth; x++)
            { 
                if (getPix(n, y, x))
                    pGFXChannel->drawPixel(panelStart + xStart+x+xOffset,y+yOffset,pGFXChannel->to16bit(color));

            }
            
        }
        
        const int PeakFadeTime_ms = 1000;

        CRGB colorHighlight = CRGB(CRGB::White);
	    unsigned long msPeakAge = millis();// - g_lastPeak1Time[iBand];
	    if (msPeakAge > PeakFadeTime_ms)
		    msPeakAge = PeakFadeTime_ms;
	    
        float agePercent = (float) msPeakAge / (float) MS_PER_SECOND;
	    uint8_t fadeAmount = std::min(255.0f, agePercent * 256);

        colorHighlight = CRGB(CRGB::White).fadeToBlackBy(fadeAmount);

        //if (value == 0)
		    colorHighlight = pGFXChannel->from16Bit(baseColor);

        // if decay rate is less than zero we interpret that here to mean "don't draw it at all".  

        //if (true) //g_peak1DecayRate >= 0.0f)
            //pGFXChannel->drawLine(xOffset, max(0, yOffset-1), xOffset + charWidth - 1, max(0, yOffset-1),pGFXChannel->to16bit(colorHighlight));
    }

  public:

    FTCPanelEffect(const char   * pszFriendlyName = nullptr, 
                           const CRGBPalette256 & palette = spectrumBasicColors, 
                           uint16_t           scrollSpeed = 0, 
                           uint8_t               fadeRate = 0,
                           float           peak1DecayRate = 2.0,
                           float           peak2DecayRate = 2.0)
        : LEDStripEffect(pszFriendlyName),
          _colorOffset(0),
          _scrollSpeed(scrollSpeed), 
          _fadeRate(fadeRate),
          _palette(palette)
    {
        //g_peak1DecayRate = peak1DecayRate;
        //g_peak2DecayRate = peak2DecayRate;
    }

    FTCPanelEffect(const char   * pszFriendlyName = nullptr, 
                           CRGB                 baseColor = 0, 
                           uint8_t               fadeRate = 0,
                           float           peak1DecayRate = 2.0,
                           float           peak2DecayRate = 2.0)
        : LEDStripEffect(pszFriendlyName),
          _colorOffset(0),
          _scrollSpeed(0), 
          _fadeRate(fadeRate),
          _palette(baseColor)
    {
    }

    void Command (char *buffer){
        //assume a well-formed buffer where the 1st byte is the Command
        uint8_t cmd = (uint8_t)buffer[0];
        switch (cmd) {
            case WRITE_SET_ALLIANCE:
                break;


            case WRITE_ALL_LED_COLOR:
                break;

            default:
                break;
                //do nothing

        }
    }
  

    virtual void Draw()
    {
        auto pGFXChannel = _GFX[0];

        if (_scrollSpeed > 0)
        {
            EVERY_N_MILLISECONDS(_scrollSpeed)
            {
                _colorOffset+=2;
            }
        }

        if (_fadeRate)
            fadeAllChannelsToBlackBy(_fadeRate);
        else
            fillSolidOnAllChannels(CRGB::Black);

  
        std::lock_guard<std::mutex> guard(Screen::_screenMutex);

        //DrawVUMeter(0);
        char teamnum[] = "reign";
        for (int i = 0; i < strlen(teamnum); i++)
        {
            CRGB bandColor = ColorFromPalette(_palette, (::map(i, 0, NUM_CHARS, 0, 255) + _colorOffset) % 256);
            DrawChar(i, pGFXChannel->to16bit(bandColor), teamnum[i],0); //multicolor palette
            DrawChar(i, pGFXChannel->to16bit(bandColor), teamnum[i],panelWidth); //multicolor palette
            //DrawBand(i, pGFXChannel->to16bit(CRGB(CRGB::Red)), teamnum[i]);
        }

        DrawAlliance(allianceIsRed, 0, 0); //panel 0
        DrawAlliance(allianceIsRed, panelWidth, 0); //panel 1

    }
};


