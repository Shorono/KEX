// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2012 Samuel Villarreal
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
//-----------------------------------------------------------------------------
//
// DESCRIPTION: Console System
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "system.h"
#include "array.h"
#include "kstring.h"
#include "client.h"
#include "keyInput.h"
#include "render.h"
#include "console.h"
#include "renderSystem.h"

kexCvar cvarDisplayConsole("con_alwaysShowConsole", CVF_BOOL|CVF_CONFIG, "0", "TODO");

kexConsole console;

//
// FCmd_ClearConsole
//

static void FCmd_ClearConsole(void) {
    console.Clear();
}

//
// kexConsole::kexConsole
//

kexConsole::kexConsole(void) {
    this->scrollBackPos     = 0;
    this->historyTop        = 0;
    this->historyCur        = -1;
    this->typeStrPos        = 0;
    this->scrollBackLines   = 0;
    this->bShiftDown        = false;
    this->bCtrlDown         = false;
    this->state             = CON_STATE_UP;
    this->blinkTime         = 0;
    this->bKeyHeld          = false;
    this->lastKeyPressed    = 0;
    this->timePressed       = 0;
    this->bShowPrompt       = true;
    this->outputLength      = 0;

    ClearOutput();
}

//
// kexConsole::~kexConsole
//

kexConsole::~kexConsole(void) {
    ClearOutput();
}

//
// kexConsole::ClearOutput
//

void kexConsole::ClearOutput(void) {
    for(int i = 0; i < CON_BUFFER_SIZE; i++) {
        memset(this->scrollBackStr[i], 0, CON_LINE_LENGTH);
        lineColor[i] = COLOR_WHITE;
    }

    scrollBackLines = 0;
}

//
// kexConsole::Clear
//

void kexConsole::Clear(void) {
    ClearOutput();
    ResetInputText();
}

//
// kexConsole::OutputTextLine
//

void kexConsole::OutputTextLine(rcolor color, const char *text) {
    if(scrollBackLines >= CON_BUFFER_SIZE) {
        for(unsigned int i = 0; i < CON_BUFFER_SIZE-1; i++) {
            memset(scrollBackStr[i], 0, CON_LINE_LENGTH);
            strcpy(scrollBackStr[i], scrollBackStr[i+1]);
            lineColor[i] = lineColor[i+1];
        }

        scrollBackLines = CON_BUFFER_SIZE-1;
    }

    unsigned int len = strlen(text);
    if(len >= CON_LINE_LENGTH)
        len = CON_LINE_LENGTH-1;

    strncpy(scrollBackStr[scrollBackLines], text, len);
    scrollBackStr[scrollBackLines][len] = '\0';
    lineColor[scrollBackLines] = color;

    scrollBackLines++;
}

//
// kexConsole::Print
//

void kexConsole::Print(rcolor color, const char *text) {
    int strLength = strlen(text);
    char *curText = (char*)text;
    char tmpChar[CON_LINE_LENGTH];

    if(cvarDeveloper.GetBool()) {
        /*memset(con_lastOutputBuffer, 0, 512);
        strcpy(con_lastOutputBuffer, kva("%f : %s",
            (sysMain.GetMS() / 1000.0f), s));*/
    }

    while(strLength > 0) {
        int lineLength = kexStr::IndexOf(curText, "\n");

        if(lineLength == -1)
            lineLength = strLength;

        strncpy(tmpChar, curText, lineLength);
        tmpChar[lineLength] = '\0';
        OutputTextLine(color, tmpChar);

        curText = (char*)&text[lineLength+1];
        strLength -= (lineLength+1);
    }
}

//
// kexConsole::LineScroll
//

void kexConsole::LineScroll(bool dir) {
    if(dir) {
        if(scrollBackPos < scrollBackLines)
            scrollBackPos++;
    }
    else {
        if(scrollBackPos > 0)
            scrollBackPos--;
    }
}

//
// kexConsole::BackSpace
//

void kexConsole::BackSpace(void) {
    if(strlen(typeStr) <= 0)
        return;

    char *trim = typeStr;
    int len = strlen(trim);

    typeStr[typeStrPos-1] = '\0';
    if(typeStrPos < len) {
        strncat(typeStr, &trim[typeStrPos], len-typeStrPos);
    }

    typeStrPos--;

    if(typeStrPos < 0)
        typeStrPos = 0;
}

//
// kexConsole::DeleteChar
//

void kexConsole::DeleteChar(void) {
    int tsLen = strlen(typeStr);

    if(tsLen > 0 && typeStrPos < tsLen) {
        char *trim = typeStr;
        int len = strlen(trim);

        typeStr[typeStrPos] = '\0';
        strncat(typeStr, &trim[typeStrPos+1], len-typeStrPos);
    }
}

//
// kexConsole::ShiftHeld
//

bool kexConsole::ShiftHeld(int c) const {
    return (c == SDLK_RSHIFT || c == SDLK_LSHIFT);
}

//
// kexConsole::MoveTypePos
//

void kexConsole::MoveTypePos(bool dir) {
    if(dir) {
        int len = strlen(typeStr);
        typeStrPos++;
        if(typeStrPos > len)
            typeStrPos = len;
    }
    else {
        typeStrPos--;
        if(typeStrPos < 0)
            typeStrPos = 0;
    }
}

//
// kexConsole::CheckShift
//

void kexConsole::CheckShift(const event_t *ev) {

    if(!ShiftHeld(ev->data1))
        return;

    switch(ev->type) {
    case ev_keydown:
        bShiftDown = true;
        break;
    case ev_keyup:
        bShiftDown = false;
        break;
    default:
        break;
    }
}

//
// kexConsole::CheckStickyKeys
//

void kexConsole::CheckStickyKeys(const event_t *ev) {
    if(ShiftHeld(ev->data1) || ev->data1 == SDLK_RETURN ||
        ev->data1 == SDLK_TAB) {
            return;
    }

    lastKeyPressed = ev->data1;

    switch(ev->type) {
    case ev_keydown:
        bKeyHeld = true;
        timePressed = sysMain.GetMS();
        break;
    case ev_keyup:
        bKeyHeld = false;
        timePressed = 0;
        break;
    default:
        break;
    }
}

//
// kexConsole::ParseKey
//

void kexConsole::ParseKey(int c) {
    switch(c) {
    case SDLK_BACKSPACE:
        BackSpace();
        return;
    case SDLK_DELETE:
        DeleteChar();
        return;
    case SDLK_LEFT:
        MoveTypePos(0);
        return;
    case SDLK_RIGHT:
        MoveTypePos(1);
        return;
    case SDLK_PAGEUP:
        LineScroll(1);
        return;
    case SDLK_PAGEDOWN:
        LineScroll(0);
        return;
    }

    if(c >= 8 && c < 256) {
        if(typeStrPos >= CON_INPUT_LENGTH)
            return;

        typeStr[typeStrPos++] = inputKey.GetAsciiKey((char)c, bShiftDown);
        typeStr[typeStrPos] = '\0';
    }
}

//
// kexConsole::StickyKeyTick
//

void kexConsole::StickyKeyTick(void) {
    if(bKeyHeld && ((sysMain.GetMS() - timePressed) >= CON_STICKY_TIME))
        ParseKey(lastKeyPressed);
}

//
// kexConsole::UpdateBlink
//

void kexConsole::UpdateBlink(void) {
    if(blinkTime >= client.GetTime())
        return;

    bShowPrompt = !bShowPrompt;
    blinkTime = client.GetTime() + CON_BLINK_TIME;
}

//
// kexConsole::ParseInput
//

void kexConsole::ParseInput(void) {
    if(typeStrPos <= 0 || strlen(typeStr) <= 0)
        return;

    OutputTextLine(RGBA(192, 192, 192, 255), typeStr);
    command.Execute(typeStr);
    ResetInputText();

    historyCur = (historyTop - 1);
}

//
// kexConsole::ProcessInput
//

bool kexConsole::ProcessInput(const event_t *ev) {
    if(ev->type == ev_mousedown || ev->type == ev_mouseup ||
        ev->type == ev_mouse) {
            return false;
    }

    if(ev->type == ev_mousewheel && state == CON_STATE_DOWN) {
        switch(ev->data1) {
        case SDL_BUTTON_WHEELUP:
            LineScroll(1);
            break;
        case SDL_BUTTON_WHEELDOWN:
            LineScroll(0);
            break;
        }

        return true;
    }

    CheckShift(ev);
    CheckStickyKeys(ev);

    int c = ev->data1;

    switch(state) {
    case CON_STATE_DOWN:
        if(ev->type == ev_keydown) {
            switch(c) {
            case SDLK_BACKQUOTE:
                state = CON_STATE_UP;
                return true;
            case SDLK_RETURN:
                ParseInput();
                return true;
            case SDLK_UP:
                return true;
            case SDLK_DOWN:
                return true;
            case SDLK_TAB:
                ParseKey(SDLK_SPACE);
                ParseKey(SDLK_SPACE);
                ParseKey(SDLK_SPACE);
                ParseKey(SDLK_SPACE);
                return true;
            default:
                ParseKey(c);
                return true;
            }

            return false;
        }
        break;
    case CON_STATE_UP:
        if(ev->type == ev_keydown) {
            switch(c) {
            case SDLK_BACKQUOTE:
                state = CON_STATE_DOWN;
                return true;
            default:
                break;
            }

            return false;
        }
        break;
    default:
        return false;
    }

    return false;
}

//
// kexConsole::Tick
//

void kexConsole::Tick(void) {
    if(state == CON_STATE_UP)
        return;

    StickyKeyTick();
    UpdateBlink();
}

//
// kexConsole::Init
//

void kexConsole::Init(void) {
    command.Add("clear", FCmd_ClearConsole);
    common.Printf("Console Initialized\n");
}

//
// kexConsole::Draw
//

void kexConsole::Draw(void) {
    static const word   drawConIndices[6] = { 0, 1, 2, 2, 1, 3 };
    static float        drawConVertices[4][3];
    static byte         drawConColors[4][4];
    bool                bOverlay;
    float               w;
    float               h;
    rcolor              color;

    if(state == CON_STATE_UP && !cvarDisplayConsole.GetBool())
        return;

    bOverlay = (state == CON_STATE_UP && cvarDisplayConsole.GetBool());

    w = (float)sysMain.VideoWidth();
    h = (float)sysMain.VideoHeight() * 0.6875f;

    dglDisableClientState(GL_NORMAL_ARRAY);
    dglEnableClientState(GL_COLOR_ARRAY);

    dglVertexPointer(3, GL_FLOAT, sizeof(float)*3, drawConVertices);
    dglColorPointer(4, GL_UNSIGNED_BYTE, sizeof(byte)*4, drawConColors);

    renderSystem.SetState(GLSTATE_BLEND, true);

    if(!bOverlay) {
        dglDisableClientState(GL_TEXTURE_COORD_ARRAY);
        
        // draw tint overlay
        for(int i = 0; i < 4; i++) {
            drawConColors[i][0] = 4;
            drawConColors[i][1] = 8;
            drawConColors[i][2] = 16;
            drawConColors[i][3] = 192;
        }

        drawConVertices[0][0] = 0;
        drawConVertices[0][1] = 0;
        drawConVertices[1][0] = w;
        drawConVertices[1][1] = 0;
        drawConVertices[2][0] = 0;
        drawConVertices[2][1] = h;
        drawConVertices[3][0] = w;
        drawConVertices[3][1] = h;

        renderSystem.whiteTexture.Bind();

        dglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, drawConIndices);

        for(int i = 0; i < 4; i++) {
            drawConColors[i][0] = 0;
            drawConColors[i][1] = 128;
            drawConColors[i][2] = 255;
            drawConColors[i][3] = 255;
        }

        // draw top border
        drawConVertices[0][1] = h-17;
        drawConVertices[1][1] = h-17;
        drawConVertices[2][1] = h-16;
        drawConVertices[3][1] = h-16;
        dglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, drawConIndices);

        // draw bottom border
        drawConVertices[0][1] = h;
        drawConVertices[1][1] = h;
        drawConVertices[2][1] = h+1;
        drawConVertices[3][1] = h+1;
        dglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, drawConIndices);
        
        dglEnableClientState(GL_TEXTURE_COORD_ARRAY);

        color = RGBA(255, 255, 255, 255);
        renderSystem.consoleFont.DrawString("> ", 0, h-15, 1, false, (byte*)&color, (byte*)&color);

        if(bShowPrompt) {
            renderSystem.consoleFont.DrawString("_", 16 +
                renderSystem.consoleFont.StringWidth(typeStr, 1.0f, typeStrPos),
                h-15, 1, false, (byte*)&color, (byte*)&color);
        }

        if(strlen(typeStr) > 0) {
            renderSystem.consoleFont.DrawString(typeStr, 16, h-15, 1, false,
                (byte*)&color, (byte*)&color);
        }
    }

    if(scrollBackLines > 0) {
        float scy = h-34;

        for(int i = scrollBackLines-(scrollBackPos)-1; i >= 0; i--) {
            if(scy < 0) {
                break;
            }

            color = lineColor[i];
            renderSystem.consoleFont.DrawString(scrollBackStr[i], 0, scy,
                1, false, (byte*)&color, (byte*)&color);
            scy -= 16;
        }
    }

    renderSystem.SetState(GLSTATE_BLEND, false);
    
    dglDisableClientState(GL_COLOR_ARRAY);
    dglEnableClientState(GL_NORMAL_ARRAY);
}
