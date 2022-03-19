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

    CRGBPalette256 _palette;

    // DrawChar
    //
    // Draws a given character from the IronFont

    void DrawChar(byte iBand, uint16_t baseColor, char n, int panel)
    {
        auto pGFXChannel = _GFX[0];

        
        int charWidth = 4; 
        int xOffset   = iBand * (charWidth +1);
        int yOffset   = 0;
        int xStart = 7;
        int panelStart = 0;

        if (panel==1) panelStart=32;

        int iRow = 0;
        CRGB color = pGFXChannel->from16Bit(baseColor);
        //pGFXChannel->fillCircle(3,3,3, CRGB(CRGB::BLUE16));
        pGFXChannel->fillRect(panelStart,0,panelStart+6,6, pGFXChannel->to16bit(CRGB(CRGB::Red)));
                
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
	    byte fadeAmount = std::min(255.0f, agePercent * 256);

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
        char teamnum[] = " 6832";
        for (int i = 0; i < strlen(teamnum); i++)
        {
            CRGB bandColor = ColorFromPalette(_palette, (::map(i, 0, NUM_BANDS, 0, 255) + _colorOffset) % 256);
            DrawChar(i, pGFXChannel->to16bit(bandColor), teamnum[i],0); //multicolor palette
            //DrawChar(i, pGFXChannel->to16bit(bandColor), teamnum[i],1); //multicolor palette
            //DrawBand(i, pGFXChannel->to16bit(CRGB(CRGB::Red)), teamnum[i]);
        }
    }
};


