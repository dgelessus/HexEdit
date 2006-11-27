// HexEditView.cpp : implementation of the CHexEditView class
//
// Copyright (c) 2003 by Andrew W. Phillips.
//
// No restrictions are placed on the noncommercial use of this code,
// as long as this text (from the above copyright notice to the
// disclaimer below) is preserved.
//
// This code may be redistributed as long as it remains unmodified
// and is not sold for profit without the author's written consent.
//
// This code, or any part of it, may not be used in any software that
// is sold for profit, without the author's written consent.
//
// DISCLAIMER: This file is provided "as is" with no expressed or
// implied warranty. The author accepts no liability for any damage
// or loss of business that this product may cause.
//

#include "stdafx.h"
#include "HexEdit.h"
#include "HexFileList.h"

#include "HexEditDoc.h"
#include "HexEditView.h"
#include "DataFormatView.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "Dialog.h"
#include "Bookmark.h"
#include "NewFile.h"
#include "Password.h"

#include "Boyer.h"
#include "SystemSound.h"
#include "Misc.h"
#include "BCGMisc.h"
#include "SRecord.h"      // For import/export of Motorola S record files
#include "IntelHex.h"     // For import/export of Intel Hex files
#include "CopyCSrc.h"     // For Copy as C Source dialog
#include "zlib/zlib.h"    // For compression
#include "md5.h"          // For MD5 hash
#include "HelpID.hm"

#ifdef _DEBUG
#include "timer.h"
#endif

#define MAX_CLIPBOARD 16000000L  // Putting more than this on the clipboard (Win 95/98) causes crash

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CHexEditApp theApp;

BYTE CHexEditFontCombo::saved_charset = 147;  // (see BCGMisc.h) initially 147 to no match 
                                              // anything -> force initial rebuild of the font list

/////////////////////////////////////////////////////////////////////////////
// CHexEditView

IMPLEMENT_DYNCREATE(CHexEditView, CScrView)

BEGIN_MESSAGE_MAP(CHexEditView, CScrView)

        //{{AFX_MSG_MAP(CHexEditView)
        ON_WM_SIZE()
        ON_COMMAND(ID_ADDR_TOGGLE, OnAddrToggle)
        ON_UPDATE_COMMAND_UI(ID_ADDR_TOGGLE, OnUpdateAddrToggle)
        ON_COMMAND(ID_GRAPHIC_TOGGLE, OnGraphicToggle)
        ON_UPDATE_COMMAND_UI(ID_GRAPHIC_TOGGLE, OnUpdateGraphicToggle)
        ON_COMMAND(ID_CHAR_TOGGLE, OnCharToggle)
        ON_UPDATE_COMMAND_UI(ID_CHAR_TOGGLE, OnUpdateCharToggle)
        ON_COMMAND(ID_FONT, OnFont)
        ON_COMMAND(ID_AUTOFIT, OnAutoFit)
        ON_UPDATE_COMMAND_UI(ID_AUTOFIT, OnUpdateAutofit)
        ON_COMMAND(ID_ASC_EBC, OnAscEbc)
        ON_UPDATE_COMMAND_UI(ID_ASC_EBC, OnUpdateAscEbc)
        ON_COMMAND(ID_CONTROL, OnControl)
        ON_UPDATE_COMMAND_UI(ID_CONTROL, OnUpdateControl)
        ON_WM_CHAR()
        ON_WM_SETFOCUS()
        ON_WM_KILLFOCUS()
        ON_WM_DESTROY()
        ON_WM_LBUTTONDOWN()
        ON_COMMAND(ID_MARK, OnMark)
        ON_COMMAND(ID_GOTO_MARK, OnGotoMark)
        ON_WM_LBUTTONDBLCLK()
        ON_WM_KEYDOWN()
        ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
        ON_COMMAND(ID_SEARCH_HEX, OnSearchHex)
        ON_COMMAND(ID_SEARCH_ASCII, OnSearchAscii)
        ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
        ON_COMMAND(ID_ALLOW_MODS, OnAllowMods)
        ON_UPDATE_COMMAND_UI(ID_ALLOW_MODS, OnUpdateAllowMods)
        ON_COMMAND(ID_CONTROL_TOGGLE, OnControlToggle)
        ON_COMMAND(ID_INSERT, OnInsert)
        ON_UPDATE_COMMAND_UI(ID_INSERT, OnUpdateInsert)
        ON_COMMAND(ID_SEARCH_ICASE, OnSearchIcase)
        ON_COMMAND(ID_EDIT_COMPARE, OnEditCompare)
        ON_COMMAND(ID_WINDOW_NEXT, OnWindowNext)
        ON_UPDATE_COMMAND_UI(ID_EDIT_COMPARE, OnUpdateEditCompare)
        ON_WM_CONTEXTMENU()
        ON_WM_RBUTTONDOWN()
        ON_COMMAND(ID_INC_BYTE, OnIncByte)
        ON_COMMAND(ID_INC_16BIT, OnInc16bit)
        ON_COMMAND(ID_INC_32BIT, OnInc32bit)
        ON_COMMAND(ID_INC_64BIT, OnInc64bit)
        ON_COMMAND(ID_DEC_BYTE, OnDecByte)
        ON_COMMAND(ID_DEC_16BIT, OnDec16bit)
        ON_COMMAND(ID_DEC_32BIT, OnDec32bit)
        ON_COMMAND(ID_DEC_64BIT, OnDec64bit)
        ON_COMMAND(ID_FLIP_16BIT, OnFlip16bit)
        ON_COMMAND(ID_FLIP_32BIT, OnFlip32bit)
        ON_COMMAND(ID_FLIP_64BIT, OnFlip64bit)
        ON_UPDATE_COMMAND_UI(ID_INC_BYTE, OnUpdateByte)
        ON_UPDATE_COMMAND_UI(ID_INC_16BIT, OnUpdate16bit)
        ON_UPDATE_COMMAND_UI(ID_INC_32BIT, OnUpdate32bit)
        ON_UPDATE_COMMAND_UI(ID_INC_64BIT, OnUpdate64bit)
        ON_WM_LBUTTONUP()
        ON_COMMAND(ID_SELECT_ALL, OnSelectAll)
        ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
        ON_COMMAND(ID_EDIT_CUT, OnEditCut)
        ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
        ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateTextPaste)
        ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateClipboard)
        ON_UPDATE_COMMAND_UI(ID_PASTE_UNICODE, OnUpdateUnicodePaste)
        ON_WM_SETCURSOR()
        ON_COMMAND(ID_FONT_DEC, OnFontDec)
        ON_COMMAND(ID_FONT_INC, OnFontInc)
        ON_UPDATE_COMMAND_UI(ID_FONT_DEC, OnUpdateFontDec)
        ON_UPDATE_COMMAND_UI(ID_FONT_INC, OnUpdateFontInc)
        ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
        ON_COMMAND(ID_PASTE_ASCII, OnPasteAscii)
        ON_COMMAND(ID_PASTE_EBCDIC, OnPasteEbcdic)
        ON_COMMAND(ID_PASTE_UNICODE, OnPasteUnicode)
        ON_COMMAND(ID_COPY_CCHAR, OnCopyCchar)
        ON_COMMAND(ID_COPY_HEX, OnCopyHex)
        ON_COMMAND(ID_EDIT_WRITEFILE, OnEditWriteFile)
        ON_UPDATE_COMMAND_UI(ID_EDIT_READFILE, OnUpdateReadFile)
        ON_COMMAND(ID_EDIT_READFILE, OnReadFile)
        ON_WM_HSCROLL()
        ON_WM_VSCROLL()
        ON_COMMAND(ID_EXTENDTO_MARK, OnExtendToMark)
        ON_COMMAND(ID_SWAP_MARK, OnSwapMark)
        ON_COMMAND(ID_REDRAW, OnRedraw)
        ON_COMMAND(ID_SCROLL_DOWN, OnScrollDown)
        ON_COMMAND(ID_SCROLL_UP, OnScrollUp)
        ON_COMMAND(ID_SWAP, OnSwap)
        ON_COMMAND(ID_START_LINE, OnStartLine)
        ON_COMMAND(ID_DEL, OnDel)
        ON_UPDATE_COMMAND_UI(ID_SWAP, OnUpdateSwap)
        ON_COMMAND(ID_OEM_TOGGLE, OnOemToggle)
        ON_UPDATE_COMMAND_UI(ID_OEM_TOGGLE, OnUpdateOemToggle)
        ON_WM_MOUSEWHEEL()
        ON_COMMAND(ID_INVERT, OnInvert)
        ON_COMMAND(ID_NEG_BYTE, OnNegByte)
        ON_COMMAND(ID_NEG_16BIT, OnNeg16bit)
        ON_COMMAND(ID_NEG_32BIT, OnNeg32bit)
        ON_COMMAND(ID_NEG_64BIT, OnNeg64bit)
        ON_WM_MOUSEMOVE()
        ON_WM_KEYUP()
        ON_COMMAND(ID_HIGHLIGHT, OnHighlight)
        ON_UPDATE_COMMAND_UI(ID_HIGHLIGHT, OnUpdateHighlight)
        ON_COMMAND(ID_HIGHLIGHT_CLEAR, OnHighlightClear)
        ON_COMMAND(ID_HIGHLIGHT_PREV, OnHighlightPrev)
        ON_COMMAND(ID_HIGHLIGHT_NEXT, OnHighlightNext)
        ON_UPDATE_COMMAND_UI(ID_HIGHLIGHT_PREV, OnUpdateHighlightPrev)
        ON_UPDATE_COMMAND_UI(ID_HIGHLIGHT_NEXT, OnUpdateHighlightNext)
        ON_COMMAND(ID_EDIT_GOTO, OnEditGoto)
    ON_COMMAND(ID_ASC2EBC, OnAscii2Ebcdic)
    ON_UPDATE_COMMAND_UI(ID_ASC2EBC, OnUpdateConvert)
    ON_COMMAND(ID_EBC2ASC, OnEbcdic2Ascii)
    ON_COMMAND(ID_ANSI2IBM, OnAnsi2Ibm)
    ON_COMMAND(ID_IBM2ANSI, OnIbm2Ansi)
    ON_COMMAND(ID_ENCRYPT_ENCRYPT, OnEncrypt)
    ON_COMMAND(ID_ENCRYPT_DECRYPT, OnDecrypt)
    ON_UPDATE_COMMAND_UI(ID_ENCRYPT_ENCRYPT, OnUpdateEncrypt)
    ON_COMMAND(ID_EDIT_APPENDFILE, OnEditAppendFile)
    ON_COMMAND(ID_EDIT_APPENDSAMEFILE, OnEditAppendSameFile)
    ON_UPDATE_COMMAND_UI(ID_EDIT_APPENDSAMEFILE, OnUpdateEditAppendSameFile)
    ON_COMMAND(ID_UNDO_CHANGES, OnUndoChanges)
    ON_UPDATE_COMMAND_UI(ID_UNDO_CHANGES, OnUpdateUndoChanges)
    ON_COMMAND(ID_CALC_SEL, OnCalcSel)
    ON_UPDATE_COMMAND_UI(ID_CALC_SEL, OnUpdateCalcSel)
    ON_COMMAND(ID_DISPLAY_RESET, OnDisplayReset)
    ON_WM_ERASEBKGND()
    ON_COMMAND(ID_IMPORT_MOTOROLA_S, OnImportMotorolaS)
    ON_UPDATE_COMMAND_UI(ID_IMPORT_MOTOROLA_S, OnUpdateImportMotorolaS)
    ON_COMMAND(ID_IMPORT_INTEL, OnImportIntel)
    ON_UPDATE_COMMAND_UI(ID_IMPORT_INTEL, OnUpdateImportIntel)
    ON_COMMAND(ID_EXPORT_INTEL, OnExportIntel)
    ON_UPDATE_COMMAND_UI(ID_EXPORT_INTEL, OnUpdateExportIntel)
    ON_COMMAND(ID_IMPORT_HEX_TEXT, OnImportHexText)
    ON_UPDATE_COMMAND_UI(ID_IMPORT_HEX_TEXT, OnUpdateImportHexText)
    ON_COMMAND(ID_EXPORT_HEX_TEXT, OnExportHexText)
    ON_UPDATE_COMMAND_UI(ID_EXPORT_HEX_TEXT, OnUpdateExportHexText)
    ON_COMMAND(ID_CRC32, OnCrc32)
    ON_COMMAND(ID_CRC_CCITT, OnCrcCcitt)
    ON_COMMAND(ID_BOOKMARKS_HIDE, OnBookmarksHide)
    ON_UPDATE_COMMAND_UI(ID_BOOKMARKS_HIDE, OnUpdateBookmarksHide)
    ON_COMMAND(ID_HIGHLIGHT_HIDE, OnHighlightHide)
    ON_UPDATE_COMMAND_UI(ID_HIGHLIGHT_HIDE, OnUpdateHighlightHide)
    ON_COMMAND(ID_BOOKMARKS_NEXT, OnBookmarksNext)
    ON_UPDATE_COMMAND_UI(ID_BOOKMARKS_NEXT, OnUpdateBookmarksNext)
    ON_COMMAND(ID_BOOKMARKS_PREV, OnBookmarksPrev)
    ON_UPDATE_COMMAND_UI(ID_BOOKMARKS_PREV, OnUpdateBookmarksPrev)
    ON_COMMAND(ID_BOOKMARKS_CLEAR, OnBookmarksClear)
    ON_UPDATE_COMMAND_UI(ID_BOOKMARKS_CLEAR, OnUpdateBookmarksClear)
    ON_COMMAND(ID_EDIT_FIND, OnEditFind)
    ON_COMMAND(ID_BOOKMARK_TOGGLE, OnBookmarkToggle)
    ON_COMMAND(ID_COLUMN_DEC, OnColumnDec)
    ON_UPDATE_COMMAND_UI(ID_COLUMN_DEC, OnUpdateColumnDec)
    ON_COMMAND(ID_COLUMN_INC, OnColumnInc)
    ON_UPDATE_COMMAND_UI(ID_COLUMN_INC, OnUpdateColumnInc)
    ON_COMMAND(ID_DFFD_AUTO_SYNC, OnDffdAutoSync)
    ON_UPDATE_COMMAND_UI(ID_DFFD_AUTO_SYNC, OnUpdateDffdAutoSync)
    ON_COMMAND(ID_DFFD_SYNC, OnDffdSync)
    ON_UPDATE_COMMAND_UI(ID_DFFD_SYNC, OnUpdateDffdSync)
    ON_COMMAND(ID_RAND_SEED_CALC, OnSeedCalc)
    ON_COMMAND(ID_RAND_SEED, OnSeedRandom)
        ON_UPDATE_COMMAND_UI(ID_CONTROL_TOGGLE, OnUpdateControl)
        ON_UPDATE_COMMAND_UI(ID_DEC_BYTE, OnUpdateByte)
        ON_UPDATE_COMMAND_UI(ID_DEC_16BIT, OnUpdate16bit)
        ON_UPDATE_COMMAND_UI(ID_DEC_32BIT, OnUpdate32bit)
        ON_UPDATE_COMMAND_UI(ID_DEC_64BIT, OnUpdate64bit)
        ON_UPDATE_COMMAND_UI(ID_FLIP_16BIT, OnUpdate16bit)
        ON_UPDATE_COMMAND_UI(ID_FLIP_32BIT, OnUpdate32bit)
        ON_UPDATE_COMMAND_UI(ID_FLIP_64BIT, OnUpdate64bit)
        ON_UPDATE_COMMAND_UI(ID_SEARCH_SEL, OnUpdateClipboard)
        ON_UPDATE_COMMAND_UI(ID_PASTE_ASCII, OnUpdateTextPaste)
        ON_UPDATE_COMMAND_UI(ID_PASTE_EBCDIC, OnUpdateTextPaste)
        ON_UPDATE_COMMAND_UI(ID_EDIT_WRITEFILE, OnUpdateClipboard)
        ON_UPDATE_COMMAND_UI(ID_INVERT, OnUpdateByte)
        ON_UPDATE_COMMAND_UI(ID_NEG_BYTE, OnUpdateByte)
        ON_UPDATE_COMMAND_UI(ID_NEG_16BIT, OnUpdate16bit)
        ON_UPDATE_COMMAND_UI(ID_NEG_32BIT, OnUpdate32bit)
        ON_UPDATE_COMMAND_UI(ID_NEG_64BIT, OnUpdate64bit)
    ON_UPDATE_COMMAND_UI(ID_EBC2ASC, OnUpdateConvert)
    ON_UPDATE_COMMAND_UI(ID_ANSI2IBM, OnUpdateConvert)
    ON_UPDATE_COMMAND_UI(ID_IBM2ANSI, OnUpdateConvert)
    ON_UPDATE_COMMAND_UI(ID_ENCRYPT_DECRYPT, OnUpdateEncrypt)
    ON_UPDATE_COMMAND_UI(ID_EDIT_APPENDFILE, OnUpdateClipboard)
    ON_UPDATE_COMMAND_UI(ID_CRC32, OnUpdateByteNZ)
    ON_UPDATE_COMMAND_UI(ID_CRC_CCITT, OnUpdateByteNZ)
    ON_UPDATE_COMMAND_UI(ID_COPY_CCHAR, OnUpdateClipboard)
    ON_UPDATE_COMMAND_UI(ID_COPY_HEX, OnUpdateClipboard)
    ON_COMMAND(ID_TRACK_CHANGES, OnTrackChanges)
    ON_UPDATE_COMMAND_UI(ID_TRACK_CHANGES, OnUpdateTrackChanges)
    //}}AFX_MSG_MAP

        ON_COMMAND(ID_EDIT_REPLACE, OnEditReplace)
        ON_COMMAND(ID_CHECKSUM8, OnChecksum8)
        ON_COMMAND(ID_CHECKSUM16, OnChecksum16)
        ON_COMMAND(ID_CHECKSUM32, OnChecksum32)
        ON_COMMAND(ID_CHECKSUM64, OnChecksum64)
        ON_UPDATE_COMMAND_UI(ID_CHECKSUM8, OnUpdateByteNZ)
        ON_UPDATE_COMMAND_UI(ID_CHECKSUM16, OnUpdate16bitNZ)
        ON_UPDATE_COMMAND_UI(ID_CHECKSUM32, OnUpdate32bitNZ)
        ON_UPDATE_COMMAND_UI(ID_CHECKSUM64, OnUpdate64bitNZ)

        ON_COMMAND(ID_XOR_BYTE, OnXorByte)
        ON_COMMAND(ID_XOR_16BIT, OnXor16bit)
        ON_COMMAND(ID_XOR_32BIT, OnXor32bit)
        ON_COMMAND(ID_XOR_64BIT, OnXor64bit)
        ON_UPDATE_COMMAND_UI(ID_XOR_BYTE, OnUpdateByteBinary)
        ON_UPDATE_COMMAND_UI(ID_XOR_16BIT, OnUpdate16bitBinary)
        ON_UPDATE_COMMAND_UI(ID_XOR_32BIT, OnUpdate32bitBinary)
        ON_UPDATE_COMMAND_UI(ID_XOR_64BIT, OnUpdate64bitBinary)
        ON_COMMAND(ID_ASSIGN_BYTE, OnAssignByte)
        ON_COMMAND(ID_ASSIGN_16BIT, OnAssign16bit)
        ON_COMMAND(ID_ASSIGN_32BIT, OnAssign32bit)
        ON_COMMAND(ID_ASSIGN_64BIT, OnAssign64bit)
        ON_UPDATE_COMMAND_UI(ID_ASSIGN_BYTE, OnUpdateByteBinary)
        ON_UPDATE_COMMAND_UI(ID_ASSIGN_16BIT, OnUpdate16bitBinary)
        ON_UPDATE_COMMAND_UI(ID_ASSIGN_32BIT, OnUpdate32bitBinary)
        ON_UPDATE_COMMAND_UI(ID_ASSIGN_64BIT, OnUpdate64bitBinary)
        ON_COMMAND(ID_RAND_BYTE, OnRandByte)
        ON_COMMAND(ID_RAND_FAST, OnRandFast)
//        ON_COMMAND(ID_RAND_16BIT, OnRand16bit)
//        ON_COMMAND(ID_RAND_32BIT, OnRand32bit)
//        ON_COMMAND(ID_RAND_64BIT, OnRand64bit)
        ON_UPDATE_COMMAND_UI(ID_RAND_BYTE, OnUpdateByte)
        ON_UPDATE_COMMAND_UI(ID_RAND_FAST, OnUpdateByte)
//        ON_UPDATE_COMMAND_UI(ID_RAND_16BIT, OnUpdate16bit)
//        ON_UPDATE_COMMAND_UI(ID_RAND_32BIT, OnUpdate32bit)
//        ON_UPDATE_COMMAND_UI(ID_RAND_64BIT, OnUpdate64bit)
        ON_COMMAND(ID_ADD_BYTE, OnAddByte)
        ON_COMMAND(ID_ADD_16BIT, OnAdd16bit)
        ON_COMMAND(ID_ADD_32BIT, OnAdd32bit)
        ON_COMMAND(ID_ADD_64BIT, OnAdd64bit)
        ON_UPDATE_COMMAND_UI(ID_ADD_BYTE, OnUpdateByteBinary)
        ON_UPDATE_COMMAND_UI(ID_ADD_16BIT, OnUpdate16bitBinary)
        ON_UPDATE_COMMAND_UI(ID_ADD_32BIT, OnUpdate32bitBinary)
        ON_UPDATE_COMMAND_UI(ID_ADD_64BIT, OnUpdate64bitBinary)
        ON_COMMAND(ID_SUBTRACT_BYTE, OnSubtractByte)
        ON_COMMAND(ID_SUBTRACT_16BIT, OnSubtract16bit)
        ON_COMMAND(ID_SUBTRACT_32BIT, OnSubtract32bit)
        ON_COMMAND(ID_SUBTRACT_64BIT, OnSubtract64bit)
        ON_UPDATE_COMMAND_UI(ID_SUBTRACT_BYTE, OnUpdateByteBinary)
        ON_UPDATE_COMMAND_UI(ID_SUBTRACT_16BIT, OnUpdate16bitBinary)
        ON_UPDATE_COMMAND_UI(ID_SUBTRACT_32BIT, OnUpdate32bitBinary)
        ON_UPDATE_COMMAND_UI(ID_SUBTRACT_64BIT, OnUpdate64bitBinary)
        ON_COMMAND(ID_AND_BYTE, OnAndByte)
        ON_COMMAND(ID_AND_16BIT, OnAnd16bit)
        ON_COMMAND(ID_AND_32BIT, OnAnd32bit)
        ON_COMMAND(ID_AND_64BIT, OnAnd64bit)
        ON_UPDATE_COMMAND_UI(ID_AND_BYTE, OnUpdateByteBinary)
        ON_UPDATE_COMMAND_UI(ID_AND_16BIT, OnUpdate16bitBinary)
        ON_UPDATE_COMMAND_UI(ID_AND_32BIT, OnUpdate32bitBinary)
        ON_UPDATE_COMMAND_UI(ID_AND_64BIT, OnUpdate64bitBinary)
        ON_COMMAND(ID_OR_BYTE, OnOrByte)
        ON_COMMAND(ID_OR_16BIT, OnOr16bit)
        ON_COMMAND(ID_OR_32BIT, OnOr32bit)
        ON_COMMAND(ID_OR_64BIT, OnOr64bit)
        ON_UPDATE_COMMAND_UI(ID_OR_BYTE, OnUpdateByteBinary)
        ON_UPDATE_COMMAND_UI(ID_OR_16BIT, OnUpdate16bitBinary)
        ON_UPDATE_COMMAND_UI(ID_OR_32BIT, OnUpdate32bitBinary)
        ON_UPDATE_COMMAND_UI(ID_OR_64BIT, OnUpdate64bitBinary)

        ON_COMMAND(ID_MUL_BYTE, OnMulByte)
        ON_UPDATE_COMMAND_UI(ID_MUL_BYTE, OnUpdateByteBinary)
        ON_COMMAND(ID_MUL_16BIT, OnMul16bit)
        ON_UPDATE_COMMAND_UI(ID_MUL_16BIT, OnUpdate16bitBinary)
        ON_COMMAND(ID_MUL_32BIT, OnMul32bit)
        ON_UPDATE_COMMAND_UI(ID_MUL_32BIT, OnUpdate32bitBinary)
        ON_COMMAND(ID_MUL_64BIT, OnMul64bit)
        ON_UPDATE_COMMAND_UI(ID_MUL_64BIT, OnUpdate64bitBinary)
        ON_COMMAND(ID_DIV_BYTE, OnDivByte)
        ON_UPDATE_COMMAND_UI(ID_DIV_BYTE, OnUpdateByteBinary)
        ON_COMMAND(ID_DIV_16BIT, OnDiv16bit)
        ON_UPDATE_COMMAND_UI(ID_DIV_16BIT, OnUpdate16bitBinary)
        ON_COMMAND(ID_DIV_32BIT, OnDiv32bit)
        ON_UPDATE_COMMAND_UI(ID_DIV_32BIT, OnUpdate32bitBinary)
        ON_COMMAND(ID_DIV_64BIT, OnDiv64bit)
        ON_UPDATE_COMMAND_UI(ID_DIV_64BIT, OnUpdate64bitBinary)
        ON_COMMAND(ID_MOD_BYTE, OnModByte)
        ON_UPDATE_COMMAND_UI(ID_MOD_BYTE, OnUpdateByteBinary)
        ON_COMMAND(ID_MOD_16BIT, OnMod16bit)
        ON_UPDATE_COMMAND_UI(ID_MOD_16BIT, OnUpdate16bitBinary)
        ON_COMMAND(ID_MOD_32BIT, OnMod32bit)
        ON_UPDATE_COMMAND_UI(ID_MOD_32BIT, OnUpdate32bitBinary)
        ON_COMMAND(ID_MOD_64BIT, OnMod64bit)
        ON_UPDATE_COMMAND_UI(ID_MOD_64BIT, OnUpdate64bitBinary)
        ON_COMMAND(ID_REV_BYTE, OnRevByte)
        ON_UPDATE_COMMAND_UI(ID_REV_BYTE, OnUpdateByte)
        ON_COMMAND(ID_REV_16BIT, OnRev16bit)
        ON_UPDATE_COMMAND_UI(ID_REV_16BIT, OnUpdate16bit)
        ON_COMMAND(ID_REV_32BIT, OnRev32bit)
        ON_UPDATE_COMMAND_UI(ID_REV_32BIT, OnUpdate32bit)
        ON_COMMAND(ID_REV_64BIT, OnRev64bit)
        ON_UPDATE_COMMAND_UI(ID_REV_64BIT, OnUpdate64bit)
        ON_COMMAND(ID_SUBTRACT_X_BYTE, OnSubtractXByte)
        ON_UPDATE_COMMAND_UI(ID_SUBTRACT_X_BYTE, OnUpdateByteBinary)
        ON_COMMAND(ID_SUBTRACT_X_16BIT, OnSubtractX16bit)
        ON_UPDATE_COMMAND_UI(ID_SUBTRACT_X_16BIT, OnUpdate16bitBinary)
        ON_COMMAND(ID_SUBTRACT_X_32BIT, OnSubtractX32bit)
        ON_UPDATE_COMMAND_UI(ID_SUBTRACT_X_32BIT, OnUpdate32bitBinary)
        ON_COMMAND(ID_SUBTRACT_X_64BIT, OnSubtractX64bit)
        ON_UPDATE_COMMAND_UI(ID_SUBTRACT_X_64BIT, OnUpdate64bitBinary)
        ON_COMMAND(ID_DIV_X_BYTE, OnDivXByte)
        ON_UPDATE_COMMAND_UI(ID_DIV_X_BYTE, OnUpdateByteBinary)
        ON_COMMAND(ID_DIV_X_16BIT, OnDivX16bit)
        ON_UPDATE_COMMAND_UI(ID_DIV_X_16BIT, OnUpdate16bitBinary)
        ON_COMMAND(ID_DIV_X_32BIT, OnDivX32bit)
        ON_UPDATE_COMMAND_UI(ID_DIV_X_32BIT, OnUpdate32bitBinary)
        ON_COMMAND(ID_DIV_X_64BIT, OnDivX64bit)
        ON_UPDATE_COMMAND_UI(ID_DIV_X_64BIT, OnUpdate64bitBinary)
        ON_COMMAND(ID_MOD_X_BYTE, OnModXByte)
        ON_UPDATE_COMMAND_UI(ID_MOD_X_BYTE, OnUpdateByteBinary)
        ON_COMMAND(ID_MOD_X_16BIT, OnModX16bit)
        ON_UPDATE_COMMAND_UI(ID_MOD_X_16BIT, OnUpdate16bitBinary)
        ON_COMMAND(ID_MOD_X_32BIT, OnModX32bit)
        ON_UPDATE_COMMAND_UI(ID_MOD_X_32BIT, OnUpdate32bitBinary)
        ON_COMMAND(ID_MOD_X_64BIT, OnModX64bit)
        ON_UPDATE_COMMAND_UI(ID_MOD_X_64BIT, OnUpdate64bitBinary)
        ON_COMMAND(ID_GTR_BYTE, OnGtrByte)
        ON_UPDATE_COMMAND_UI(ID_GTR_BYTE, OnUpdateByteBinary)
        ON_COMMAND(ID_GTR_16BIT, OnGtr16bit)
        ON_UPDATE_COMMAND_UI(ID_GTR_16BIT, OnUpdate16bitBinary)
        ON_COMMAND(ID_GTR_32BIT, OnGtr32bit)
        ON_UPDATE_COMMAND_UI(ID_GTR_32BIT, OnUpdate32bitBinary)
        ON_COMMAND(ID_GTR_64BIT, OnGtr64bit)
        ON_UPDATE_COMMAND_UI(ID_GTR_64BIT, OnUpdate64bitBinary)
        ON_COMMAND(ID_LESS_BYTE, OnLessByte)
        ON_UPDATE_COMMAND_UI(ID_LESS_BYTE, OnUpdateByteBinary)
        ON_COMMAND(ID_LESS_16BIT, OnLess16bit)
        ON_UPDATE_COMMAND_UI(ID_LESS_16BIT, OnUpdate16bitBinary)
        ON_COMMAND(ID_LESS_32BIT, OnLess32bit)
        ON_UPDATE_COMMAND_UI(ID_LESS_32BIT, OnUpdate32bitBinary)
        ON_COMMAND(ID_LESS_64BIT, OnLess64bit)
        ON_UPDATE_COMMAND_UI(ID_LESS_64BIT, OnUpdate64bitBinary)
        ON_COMMAND(ID_GTRU_BYTE, OnGtrUByte)
        ON_UPDATE_COMMAND_UI(ID_GTRU_BYTE, OnUpdateByteBinary)
        ON_COMMAND(ID_GTRU_16BIT, OnGtrU16bit)
        ON_UPDATE_COMMAND_UI(ID_GTRU_16BIT, OnUpdate16bitBinary)
        ON_COMMAND(ID_GTRU_32BIT, OnGtrU32bit)
        ON_UPDATE_COMMAND_UI(ID_GTRU_32BIT, OnUpdate32bitBinary)
        ON_COMMAND(ID_GTRU_64BIT, OnGtrU64bit)
        ON_UPDATE_COMMAND_UI(ID_GTRU_64BIT, OnUpdate64bitBinary)
        ON_COMMAND(ID_LESSU_BYTE, OnLessUByte)
        ON_UPDATE_COMMAND_UI(ID_LESSU_BYTE, OnUpdateByteBinary)
        ON_COMMAND(ID_LESSU_16BIT, OnLessU16bit)
        ON_UPDATE_COMMAND_UI(ID_LESSU_16BIT, OnUpdate16bitBinary)
        ON_COMMAND(ID_LESSU_32BIT, OnLessU32bit)
        ON_UPDATE_COMMAND_UI(ID_LESSU_32BIT, OnUpdate32bitBinary)
        ON_COMMAND(ID_LESSU_64BIT, OnLessU64bit)
        ON_UPDATE_COMMAND_UI(ID_LESSU_64BIT, OnUpdate64bitBinary)

        // Note shifts don't care about calc operand size (bits) since values are less than 64
        ON_COMMAND(ID_ROL_BYTE, OnRolByte)
        ON_COMMAND(ID_ROL_16BIT, OnRol16bit)
        ON_COMMAND(ID_ROL_32BIT, OnRol32bit)
        ON_COMMAND(ID_ROL_64BIT, OnRol64bit)
        ON_UPDATE_COMMAND_UI(ID_ROL_BYTE, OnUpdateByte)
        ON_UPDATE_COMMAND_UI(ID_ROL_16BIT, OnUpdate16bit)
        ON_UPDATE_COMMAND_UI(ID_ROL_32BIT, OnUpdate32bit)
        ON_UPDATE_COMMAND_UI(ID_ROL_64BIT, OnUpdate64bit)
        ON_COMMAND(ID_ROR_BYTE, OnRolByte)
        ON_COMMAND(ID_ROR_16BIT, OnRol16bit)
        ON_COMMAND(ID_ROR_32BIT, OnRol32bit)
        ON_COMMAND(ID_ROR_64BIT, OnRol64bit)
        ON_UPDATE_COMMAND_UI(ID_ROR_BYTE, OnUpdateByte)
        ON_UPDATE_COMMAND_UI(ID_ROR_16BIT, OnUpdate16bit)
        ON_UPDATE_COMMAND_UI(ID_ROR_32BIT, OnUpdate32bit)
        ON_UPDATE_COMMAND_UI(ID_ROR_64BIT, OnUpdate64bit)
        ON_COMMAND(ID_LSL_BYTE, OnLslByte)
        ON_COMMAND(ID_LSL_16BIT, OnLsl16bit)
        ON_COMMAND(ID_LSL_32BIT, OnLsl32bit)
        ON_COMMAND(ID_LSL_64BIT, OnLsl64bit)
        ON_UPDATE_COMMAND_UI(ID_LSL_BYTE, OnUpdateByte)
        ON_UPDATE_COMMAND_UI(ID_LSL_16BIT, OnUpdate16bit)
        ON_UPDATE_COMMAND_UI(ID_LSL_32BIT, OnUpdate32bit)
        ON_UPDATE_COMMAND_UI(ID_LSL_64BIT, OnUpdate64bit)
        ON_COMMAND(ID_LSR_BYTE, OnLsrByte)
        ON_COMMAND(ID_LSR_16BIT, OnLsr16bit)
        ON_COMMAND(ID_LSR_32BIT, OnLsr32bit)
        ON_COMMAND(ID_LSR_64BIT, OnLsr64bit)
        ON_UPDATE_COMMAND_UI(ID_LSR_BYTE, OnUpdateByte)
        ON_UPDATE_COMMAND_UI(ID_LSR_16BIT, OnUpdate16bit)
        ON_UPDATE_COMMAND_UI(ID_LSR_32BIT, OnUpdate32bit)
        ON_UPDATE_COMMAND_UI(ID_LSR_64BIT, OnUpdate64bit)
        ON_COMMAND(ID_ASR_BYTE, OnAsrByte)
        ON_COMMAND(ID_ASR_16BIT, OnAsr16bit)
        ON_COMMAND(ID_ASR_32BIT, OnAsr32bit)
        ON_COMMAND(ID_ASR_64BIT, OnAsr64bit)
        ON_UPDATE_COMMAND_UI(ID_ASR_BYTE, OnUpdateByte)
        ON_UPDATE_COMMAND_UI(ID_ASR_16BIT, OnUpdate16bit)
        ON_UPDATE_COMMAND_UI(ID_ASR_32BIT, OnUpdate32bit)
        ON_UPDATE_COMMAND_UI(ID_ASR_64BIT, OnUpdate64bit)

    ON_COMMAND_RANGE(ID_EXPORT_S1, ID_EXPORT_S3, OnExportSRecord)
    ON_UPDATE_COMMAND_UI_RANGE(ID_EXPORT_S1, ID_EXPORT_S3, OnUpdateExportSRecord)

        ON_UPDATE_COMMAND_UI(ID_INSERT_BLOCK, OnUpdateInsertBlock)
        ON_COMMAND(ID_INSERT_BLOCK, OnInsertBlock)

        // New display handling commands
        ON_COMMAND(ID_DISPLAY_HEX, OnDisplayHex)
        ON_UPDATE_COMMAND_UI(ID_DISPLAY_HEX, OnUpdateDisplayHex)
        ON_COMMAND(ID_DISPLAY_CHAR, OnDisplayChar)
        ON_UPDATE_COMMAND_UI(ID_DISPLAY_CHAR, OnUpdateDisplayChar)
        ON_COMMAND(ID_DISPLAY_BOTH, OnDisplayBoth)
        ON_UPDATE_COMMAND_UI(ID_DISPLAY_BOTH, OnUpdateDisplayBoth)
        ON_COMMAND(ID_DISPLAY_STACKED, OnDisplayStacked)
        ON_UPDATE_COMMAND_UI(ID_DISPLAY_STACKED, OnUpdateDisplayStacked)
        ON_COMMAND(ID_CHARSET_ASCII, OnCharsetAscii)
        ON_UPDATE_COMMAND_UI(ID_CHARSET_ASCII, OnUpdateCharsetAscii)
        ON_COMMAND(ID_CHARSET_ANSI, OnCharsetAnsi)
        ON_UPDATE_COMMAND_UI(ID_CHARSET_ANSI, OnUpdateCharsetAnsi)
        ON_COMMAND(ID_CHARSET_OEM, OnCharsetOem)
        ON_UPDATE_COMMAND_UI(ID_CHARSET_OEM, OnUpdateCharsetOem)
        ON_COMMAND(ID_CHARSET_EBCDIC, OnCharsetEbcdic)
        ON_UPDATE_COMMAND_UI(ID_CHARSET_EBCDIC, OnUpdateCharsetEbcdic)
        ON_COMMAND(ID_CONTROL_NONE, OnControlNone)
        ON_UPDATE_COMMAND_UI(ID_CONTROL_NONE, OnUpdateControlNone)
        ON_COMMAND(ID_CONTROL_ALPHA, OnControlAlpha)
        ON_UPDATE_COMMAND_UI(ID_CONTROL_ALPHA, OnUpdateControlAlpha)
        ON_COMMAND(ID_CONTROL_C, OnControlC)
        ON_UPDATE_COMMAND_UI(ID_CONTROL_C, OnUpdateControlC)

        ON_COMMAND(ID_TOGGLE_ENDIAN, OnToggleEndian)
        ON_COMMAND(ID_BIG_ENDIAN, OnBigEndian)
        ON_COMMAND(ID_LITTLE_ENDIAN, OnLittleEndian)
        ON_UPDATE_COMMAND_UI(ID_TOGGLE_ENDIAN, OnUpdateToggleEndian)
        ON_UPDATE_COMMAND_UI(ID_BIG_ENDIAN, OnUpdateBigEndian)
        ON_UPDATE_COMMAND_UI(ID_LITTLE_ENDIAN, OnUpdateLittleEndian)

        // Standard printing commands
        ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
        ON_COMMAND(ID_FILE_PRINT_DIRECT, OnFilePrint)
        ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
        ON_COMMAND(ID_JUMP_HEX_ADDR, OnJumpHexAddr)

//        ON_COMMAND(ID_SEARCH_START, OnSearch)     // Handle message from BCG search combo
        ON_COMMAND(ID_JUMP_HEX_START, OnJumpHex)  // Handle message from BCG hex combo
        ON_COMMAND(ID_JUMP_DEC_START, OnJumpDec)  // Handle message from BCG dec combo

        ON_COMMAND(ID_SELECT_LINE, OnSelectLine)
        ON_COMMAND(IDC_FONTNAME, OnFontName)
        ON_UPDATE_COMMAND_UI(IDC_FONTNAME, OnUpdateFontName)
        ON_COMMAND(IDC_FONTSIZE, OnFontSize)
        ON_UPDATE_COMMAND_UI(IDC_FONTSIZE, OnUpdateFontSize)
        ON_CBN_SELENDOK(IDC_FONTNAME, OnFontName)
        ON_CBN_SELENDOK(IDC_FONTSIZE, OnFontSize)

    ON_COMMAND(ID_ZLIB_COMPRESS, OnCompress)
    ON_UPDATE_COMMAND_UI(ID_ZLIB_COMPRESS, OnUpdateSelNZ)
    ON_COMMAND(ID_ZLIB_DECOMPRESS, OnDecompress)
    ON_UPDATE_COMMAND_UI(ID_ZLIB_DECOMPRESS, OnUpdateSelNZ)
    ON_COMMAND(ID_MD5, OnMd5)
    ON_UPDATE_COMMAND_UI(ID_MD5, OnUpdateByteNZ)

    ON_COMMAND(ID_UPPERCASE, OnUppercase)
    ON_UPDATE_COMMAND_UI(ID_UPPERCASE, OnUpdateConvert)
    ON_COMMAND(ID_LOWERCASE, OnLowercase)
    ON_UPDATE_COMMAND_UI(ID_LOWERCASE, OnUpdateConvert)

    //ON_WM_TIMER()

    ON_COMMAND(ID_VIEWTEST, OnViewtest)
    ON_WM_SYSCOLORCHANGE()
	ON_MESSAGE(WM_MOUSEHOVER, OnMouseHover)
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
    END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHexEditView construction/destruction
CHexEditView::CHexEditView() : expr_(this)
{
    text_height_ = 0;     // While text_height_ == 0 none of the display settings have been calculated
    pdfv_ = NULL;

    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());

    disp_state_ = previous_state_ = 0;

    // We have to calculate these here since messages such as OnSize depend
    // on them and can be received before \() is called.
    win_size_ = null_size;
    pfont_ = NULL;
    pbrush_ = NULL;
    print_font_ = NULL;

    // Set up opening display based on global options
    rowsize_ = aa->open_rowsize_;
    real_offset_ = offset_ = aa->open_offset_;
    if (real_offset_ >= rowsize_)
        offset_ = rowsize_ - 1;         // In case soemone fiddled with the settings
    group_by_ = aa->open_group_by_;

    ASSERT(theApp.open_disp_state_ != -1);
    disp_state_ = theApp.open_disp_state_;

    bg_col_ = -1;

    hl_set_.clear();

    mark_ = 0L;                         // Mark initially at start of file
    if (!display_.hex_area)
        display_.edit_char = display_.mark_char = TRUE;  // No hex area so must use char
    else
        display_.edit_char = display_.mark_char = FALSE;    // Caret init. in hex not char area

    mouse_down_ = false;
    needs_refresh_ = false;
    needs_hscroll_ = false;
    needs_vscroll_ = false;

    memset((void *)&lf_, '\0', sizeof(lf_));
    _tcscpy(lf_.lfFaceName, _T("Courier")); // A nice fixed (no-proportional) font
    lf_.lfHeight = 16;
    lf_.lfCharSet = ANSI_CHARSET;           // Only allow ANSI character set fonts
    oem_lf_ = lf_;
    _tcscpy(oem_lf_.lfFaceName, _T("Terminal")); // The only certain OEM font?
    oem_lf_.lfHeight = 18;
    oem_lf_.lfCharSet = OEM_CHARSET;         // Only allow OEM/IBM character set fonts

    search_length_ = 0;
	last_tip_addr_ = -1;
	tip_show_bookmark_ = true;
	tip_show_error_ = true;

#ifndef NDEBUG
    // Make default capacity for undo_ vector small to force reallocation sooner.
    // This increases likelihood of catching bugs related to reallocation.
    undo_.reserve(4);
#else
    undo_.reserve(1024);
#endif

	errors_mentioned_ = false;
}

CHexEditView::~CHexEditView()
{
    if (pfont_ != NULL)
        delete pfont_;
    if (pbrush_ != NULL)
        delete pbrush_;
} 

BOOL CHexEditView::PreCreateWindow(CREATESTRUCT& cs) 
{
    BOOL retval = CScrView::PreCreateWindow(cs);

    // Get the create context so we can find out if this window is
    // being created via Window/New and the frame/view it's cloned from
    CCreateContext *pContext = (CCreateContext *)cs.lpCreateParams;

    // xxx when using CBGCTabView pContext is not passed through
    if (pContext != NULL && pContext->m_pCurrentFrame != NULL)
    {
        // Must have been created via Window/New (ID_WINDOW_NEW)
        ASSERT_KINDOF(CMDIChildWnd, pContext->m_pCurrentFrame);

        // We have the frame so get the view within it
        CHexEditView *pView = (CHexEditView *)pContext->m_pCurrentFrame->GetActiveView();
        ASSERT_KINDOF(CHexEditView, pView);

        // Make this view's undo stack the same as clone view
        undo_ = pView->undo_;

        // Flag any changes as not being made in this view
        std::vector <view_undo>::iterator pu;
        for (pu = undo_.begin(); pu != undo_.end(); ++pu)
        {
            if (pu->utype == undo_change)
                pu->flag = FALSE;
        }
    }

    return retval;
}

void CHexEditView::OnInitialUpdate() 
{
    FILE_ADDRESS start_addr = 0;
    FILE_ADDRESS end_addr = 0;
    CHexEditDoc *pDoc = GetDocument();
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());

    print_map_mode_ = MM_HIENGLISH;

    // Get options for the window from file settings in CHexFileList
    CHexFileList *pfl = aa->GetFileList();
    int recent_file_index = -1;
    if (pDoc->pfile1_ != NULL)
        recent_file_index = pfl->GetIndex(pDoc->pfile1_->GetFilePath());

    if (recent_file_index != -1)
    {
		// Create any associated "sub" views
		int width = atoi(pfl->GetData(recent_file_index, CHexFileList::DFFDVIEW));
		switch (width)
		{
		case 0:        // When last open template view was hidden so do nothing
			break;
		case 1:        // Last opened in tab view
			{
				CTabView *ptv = GetFrame()->ptv_;
				ptv->AddView(RUNTIME_CLASS (CDataFormatView), _T("Template (Tree) View"));
			    
				ptv->SetActiveView(1);
				pdfv_ = (CDataFormatView *)ptv->GetActiveView();
				ASSERT_KINDOF(CDataFormatView, pdfv_);

				ptv->SetActiveView(0);

				// Make sure dataformat view knows which hex view it is assoc. with
				pdfv_->phev_ = this;
			}
			break;
		default:       // Last opened in splitter
			{
				CCreateContext ctxt;
				ctxt.m_pNewViewClass = RUNTIME_CLASS(CDataFormatView);
				ctxt.m_pCurrentDoc = GetDocument();
				ctxt.m_pLastView = this;
				ctxt.m_pCurrentFrame = GetFrame();

				CHexEditSplitter *psplitter = &(GetFrame()->splitter_);
				ASSERT(psplitter->m_hWnd != 0);

				// Add left column for DFFD (template) view
				VERIFY(psplitter->InsColumn(0, width, RUNTIME_CLASS(CDataFormatView), &ctxt));

				psplitter->SetActivePane(0, 1);   // make hex view active

				pdfv_ = (CDataFormatView *)psplitter->GetPane(0, 0);
				ASSERT_KINDOF(CDataFormatView, pdfv_);

				// Make sure dataformat view knows which hex view it is assoc. with
				pdfv_->phev_ = this;
			}
			break;
		}

        disp_state_ = atoi(pfl->GetData(recent_file_index, CHexFileList::DISPLAY));
        SetVertBufferZone(atoi(pfl->GetData(recent_file_index, CHexFileList::VERT_BUFFER_ZONE)));

        scheme_name_ = pfl->GetData(recent_file_index, CHexFileList::SCHEME);
        if (scheme_name_.IsEmpty())
        {
            // Get file extension and change "." to "_"
            CString ss = pDoc->pfile1_->GetFileName();
            if (ss.ReverseFind('.') == -1)
                ss = "_";
            else
                ss = CString("_") + ss.Mid(ss.ReverseFind('.')+1);

            // If there is a scheme of this name make it the scheme to use
            std::vector<CScheme>::const_iterator ps;
            for (ps = theApp.scheme_.begin(); ps != theApp.scheme_.end(); ++ps)
            {
                if (ss.CompareNoCase(ps->name_) == 0)
                {
                    scheme_name_ = ps->name_;
                    break;
                }
            }
        }
        set_colours();

        rowsize_ = atoi(pfl->GetData(recent_file_index, CHexFileList::COLUMNS));
        real_offset_ = offset_ = atoi(pfl->GetData(recent_file_index, CHexFileList::OFFSET));
        if (real_offset_ >= rowsize_)
            offset_ = rowsize_ - 1;             // In case soemone fiddled with the settings
        group_by_ = atoi(pfl->GetData(recent_file_index, CHexFileList::GROUPING));

        start_addr = _atoi64(pfl->GetData(recent_file_index, CHexFileList::SELSTART));
        end_addr = _atoi64(pfl->GetData(recent_file_index, CHexFileList::SELEND));
        if (start_addr < 0 || start_addr > pDoc->length())
            start_addr = 0;
        if (end_addr < start_addr || end_addr > pDoc->length())
            end_addr = start_addr;

        mark_ = _atoi64(pfl->GetData(recent_file_index, CHexFileList::MARK));
        if (mark_ < 0 || mark_ > pDoc->length())
            mark_ = 0;
        ASSERT(display_.hex_area || display_.edit_char);  // if no hex area we must be editing chars

        // Get saved font info
        strncpy(lf_.lfFaceName, pfl->GetData(recent_file_index, CHexFileList::FONT), LF_FACESIZE-1);
        lf_.lfFaceName[LF_FACESIZE-1] = '\0';
        lf_.lfHeight = atoi(pfl->GetData(recent_file_index, CHexFileList::HEIGHT));
        strncpy(oem_lf_.lfFaceName, pfl->GetData(recent_file_index, CHexFileList::OEMFONT), LF_FACESIZE-1);
        oem_lf_.lfFaceName[LF_FACESIZE-1] = '\0';
        oem_lf_.lfHeight = atoi(pfl->GetData(recent_file_index, CHexFileList::OEMHEIGHT));

        std::istringstream strstr((const char *)pfl->GetData(recent_file_index, CHexFileList::HIGHLIGHTS));
        strstr >> hl_set_;

        CRect newpos;
        newpos.top =  atoi(pfl->GetData(recent_file_index, CHexFileList::TOP));
        newpos.left =  atoi(pfl->GetData(recent_file_index, CHexFileList::LEFT));
        newpos.bottom =  atoi(pfl->GetData(recent_file_index, CHexFileList::BOTTOM));
        newpos.right =  atoi(pfl->GetData(recent_file_index, CHexFileList::RIGHT));

        // Make sure that window size seems reasonable
        if (newpos.top != -30000 && newpos.left != -30000 &&
            newpos.top < newpos.bottom && newpos.left < newpos.right )
        {
            WINDOWPLACEMENT wp;
            wp.length = sizeof(wp);
            wp.flags = 0;
            wp.showCmd = atoi(pfl->GetData(recent_file_index, CHexFileList::CMD));
            wp.ptMinPosition = CPoint(-1,-1);
            wp.ptMaxPosition = CPoint(-1,-1);

            // If this window has sibling view this window was presumably created
            // using Window/New - make sure its not in the same place as its sibling(s).
            POSITION pos = pDoc->GetFirstViewPosition();
            while (pos != NULL)
            {
                WINDOWPLACEMENT frame_wp;
                frame_wp.length = sizeof(frame_wp);
                CView *pv = pDoc->GetNextView(pos);
                if (pv->IsKindOf(RUNTIME_CLASS(CHexEditView)))
                {
                    ASSERT(pv != NULL && ((CHexEditView *)pv)->GetFrame() != NULL);
                    if (pv != this && ((CHexEditView *)pv)->GetFrame()->GetWindowPlacement(&frame_wp))
                    {
                        // If the top left corners are about the same move the
                        // new window down and right a bit.
                        if (abs(newpos.top - frame_wp.rcNormalPosition.top) < 20 &&
                            abs(newpos.left - frame_wp.rcNormalPosition.left) < 20)
                        {
                            newpos += CSize(30, 30);
                        }
                    }
                }
            }

            // Check that new position is not completely off the left, right or bottom,
            // and that the top title bar is still visible to allow dragging.
            CRect rr;
            ASSERT(GetFrame() != NULL && GetFrame()->GetParent() != NULL);
            GetFrame()->GetParent()->GetClientRect(&rr);
            if (newpos.left > rr.right-20)
            {
                newpos.left = rr.right - (newpos.right - newpos.left);
                newpos.right = rr.right;
            }
            if (newpos.right < 20)
            {
                newpos.right -= newpos.left;
                newpos.left = 0;
            }
            if (newpos.top > rr.bottom-20)
            {
                newpos.top = rr.bottom - (newpos.bottom - newpos.top);
                newpos.bottom = rr.bottom;
            }
            if (newpos.top < -20)
            {
                newpos.bottom -= newpos.top;
                newpos.top =0;
            }

            wp.rcNormalPosition = newpos;
            GetFrame()->SetWindowPlacement(&wp);
        }
        else if (atoi(pfl->GetData(recent_file_index, CHexFileList::CMD)) == SW_SHOWMAXIMIZED)
        {
            WINDOWPLACEMENT wp;
            ASSERT(GetFrame() != NULL);
            GetFrame()->GetWindowPlacement(&wp);
            wp.showCmd = SW_SHOWMAXIMIZED;
            GetFrame()->SetWindowPlacement(&wp);
        }

        // Set horiz size, vert size, and scroll posn
        if (display_.vert_display || display_.char_area)
            SetSize(CSize(char_pos(rowsize_-1)+text_width_w_, -1));
        else
        {
            ASSERT(display_.hex_area);
            display_.hex_area = TRUE; // defensive
            SetSize(CSize(hex_pos(rowsize_-1)+2*text_width_, -1));
        }
        if (display_.vert_display)
            SetTSize(CSizeAp(-1, ((GetDocument()->length() + offset_)/rowsize_ + 1)*3));  // 3 rows of text
        else
            SetTSize(CSizeAp(-1, (GetDocument()->length() + offset_)/rowsize_ + 1));
        SetScroll(CPointAp(0, _atoi64(pfl->GetData(recent_file_index, CHexFileList::POS))));
    }
    else
    {
        scheme_name_ = "";          // Force scheme reset
        set_colours();

        if (aa->open_max_)
        {
            WINDOWPLACEMENT wp;
            ASSERT(GetFrame() != NULL);
            GetFrame()->GetWindowPlacement(&wp);
            wp.showCmd = SW_SHOWMAXIMIZED;
            GetFrame()->SetWindowPlacement(&wp);
        }

        // Set the normal and OEM graphic font
        if (aa->open_plf_ != NULL)
        {
            lf_ = *aa->open_plf_;
        }

        if (aa->open_oem_plf_ != NULL)
        {
            oem_lf_ = *aa->open_oem_plf_;
        }
    }

	// if (pfl->GetVersion() < 3)  // xxx this is not really sufficient if the file is not actually opened since the recent file list gets written back with same DISPLAY value (but new version number)
	{
		// In version 2 files there were 3 separate flags (eg when EBCDIC flag on other 2 flags ignored).
		// Now the 3 bits are used together to indicate char set but we allow for other bit patterns.
		if (display_.char_set == 2)
			display_.char_set = CHARSET_ASCII;       // ASCII:  0/2 -> 0
		else if (display_.char_set > 4)
			display_.char_set = CHARSET_EBCDIC;      // EBCDIC: 4/5/6/7 -> 4
	}
	if (display_.char_set == 7) // unused value
		display_.char_set = CHARSET_EBCDIC;      // EBCDIC: 7 -> 4

    // This is just here as a workaround for a small problem with MDItabs - when
    // the tabs are at the top then the tab bar gets slightly higher when the first
    // file is opened which mucks up a small part at the top of the MDI area.
    static bool first_time_here = true;
    if (first_time_here)
    {
        ((CMainFrame *)AfxGetMainWnd())->redraw_background();
        first_time_here = false;
    }

    if (!display_.edit_char)
        SetHorzBufferZone(2);

    // Convert font sizes to logical units
    {
        CPoint convert(0, lf_.lfHeight);
        CClientDC dc(this);
        OnPrepareDC(&dc);
        dc.DPtoLP(&convert);                    // Get screen font size in logical units
        lf_.lfHeight = convert.y;
    }
    {
        CPoint convert(0, oem_lf_.lfHeight);
        CClientDC dc(this);
        OnPrepareDC(&dc);
        dc.DPtoLP(&convert);                    // Get screen font size in logical units
        oem_lf_.lfHeight = convert.y;
    }

    // This can't be done till document available (ie. not in constructor)
    if (GetDocument()->read_only())
        display_.readonly = TRUE;

    // Set control bar buttons to display state of current options
//    load_bitmaps();

    CScrView::SetMapMode(MM_TEXT);

    ASSERT(pfont_ == NULL);
    pfont_ = new CFont;
    pfont_->CreateFontIndirect(display_.FontRequired() == FONT_OEM ? &oem_lf_ : &lf_);
    SetFont(pfont_);

    // Create brush that is XORed with selection when focus lost.  (This
    // gives a more subtle grey selection) when window does not have focus.)
    pbrush_ = new CBrush(RGB(192, 192, 192));
    SetBrush(pbrush_);

    CScrView::OnInitialUpdate();

#if 0
    if (recent_file_index != -1)
        SetScroll(CPointAp(0,pfl->scroll_[recent_file_index]));

    // What's the point of an empty read-only file
    if (GetDocument()->length() == 0 && !GetDocument()->read_only())
        display_.readonly = display_.overtype = FALSE;
#endif

    if (aa->large_cursor_)
        BlockCaret();
    else
        LineCaret();
    if (!display_.overtype || GetDocument()->length() > 0)
    {
        CaretMode();
        SetSel(addr2pos(start_addr), addr2pos(end_addr));
        show_prop();
        show_calc();
        show_pos();
    }

    ValidateScroll(GetScroll());

    // Save nav info in case we need to create a nav pt here
    nav_start_  = start_addr;
    nav_end_    = end_addr;
    nav_scroll_ = (GetScroll().y/line_height_)*rowsize_ - offset_;
    theApp.navman_.Add("Opened", get_info(), this, nav_start_, nav_end_, nav_scroll_);
}

// Update our options to the CHexFileList 
void CHexEditView::StoreOptions()
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
    CHexFileList *pfl = ((CHexEditApp *)AfxGetApp())->GetFileList();
    if (GetDocument()->pfile1_ == NULL)
        return;                                 // Not in the list if there's no file
    int ii = pfl->GetIndex(GetDocument()->pfile1_->GetFilePath());
    if (ii != -1)
    {
        // Found in the list so update all the values;
        WINDOWPLACEMENT wp;
        ASSERT(GetFrame() != NULL);
        if (GetFrame()->GetWindowPlacement(&wp))
        {
            pfl->SetData(ii, CHexFileList::CMD, wp.showCmd);
            pfl->SetData(ii, CHexFileList::TOP, wp.rcNormalPosition.top);
            pfl->SetData(ii, CHexFileList::LEFT, wp.rcNormalPosition.left);
            pfl->SetData(ii, CHexFileList::BOTTOM, wp.rcNormalPosition.bottom);
            pfl->SetData(ii, CHexFileList::RIGHT, wp.rcNormalPosition.right);
        }
        else
        {
            pfl->SetData(ii, CHexFileList::CMD, SW_SHOWNORMAL);
            pfl->SetData(ii, CHexFileList::TOP, -30000);
            pfl->SetData(ii, CHexFileList::LEFT, -30000);
            pfl->SetData(ii, CHexFileList::BOTTOM, -30000);
            pfl->SetData(ii, CHexFileList::RIGHT, -30000);
        }

        pfl->SetData(ii, CHexFileList::COLUMNS, rowsize_);
        pfl->SetData(ii, CHexFileList::GROUPING, group_by_);
        pfl->SetData(ii, CHexFileList::OFFSET, offset_);

        pfl->SetData(ii, CHexFileList::SCHEME, scheme_name_);
        pfl->SetData(ii, CHexFileList::FONT, lf_.lfFaceName);
        pfl->SetData(ii, CHexFileList::HEIGHT, lf_.lfHeight);
        pfl->SetData(ii, CHexFileList::OEMFONT, oem_lf_.lfFaceName);
        pfl->SetData(ii, CHexFileList::OEMHEIGHT, oem_lf_.lfHeight);

        pfl->SetData(ii, CHexFileList::DISPLAY, disp_state_);

        pfl->SetData(ii, CHexFileList::KEEP_TIMES, GetDocument()->keep_times());
        pfl->SetData(ii, CHexFileList::FORMAT, GetDocument()->GetFormatFileName());

        FILE_ADDRESS start_addr, end_addr;
        GetSelAddr(start_addr, end_addr);
        pfl->SetData(ii, CHexFileList::SELSTART, start_addr);
        pfl->SetData(ii, CHexFileList::SELEND, end_addr);
        pfl->SetData(ii, CHexFileList::POS, GetScroll().y);
        pfl->SetData(ii, CHexFileList::MARK, mark_);

        std::ostringstream strstr;
        strstr << hl_set_;
        pfl->SetData(ii, CHexFileList::HIGHLIGHTS, strstr.str().c_str());

        pfl->SetData(ii, CHexFileList::EDIT_TIME, __int64(GetDocument()->edit_time_.elapsed()));
        pfl->SetData(ii, CHexFileList::VIEW_TIME, __int64(GetDocument()->view_time_.elapsed()));
        pfl->SetData(ii, CHexFileList::VERT_BUFFER_ZONE, __int64(GetVertBufferZone()));

		int width = 0;         // default to data format view hidden
		if (pdfv_ != NULL)
		{
			width = 1;         // assume tab view used
			ASSERT(GetFrame()->splitter_.m_hWnd != 0);
			int idx = GetFrame()->splitter_.FindViewColumn(pdfv_->GetSafeHwnd());
			int dummy;        // ignored since we don't care about the height
			if (idx > -1)
			{
				GetFrame()->splitter_.GetColumnInfo(idx, width, dummy);
				if (width < 10) width = 10;    // Make sure it is not too narrow and reserve values 0-9 (1 = tab view)
			}
		}
        pfl->SetData(ii, CHexFileList::DFFDVIEW, __int64(width));
    }
}

void CHexEditView::SetScheme(const char *name)
{
    CString previous_name = scheme_name_;
    scheme_name_ = name;
    if (set_colours())
    {
        // Scheme colours set so save for undo/macros
        undo_.push_back(view_undo(undo_scheme));      // Allow undo of scheme change
        undo_.back().pscheme_name = new CString;
        *undo_.back().pscheme_name = previous_name;
        theApp.SaveToMacro(km_scheme, name);
    }
    else
    {
        // Scheme was not found (presumably in macro playback)
        ASSERT(theApp.playing_);
        CString mess;
        mess.Format("The color scheme \"%s\" was not found.\n", name);
        ::HMessageBox(mess);
        theApp.mac_error_ = 1;
        scheme_name_ = previous_name;
    }
    DoInvalidate();
}

// Gets all colour info from the app's scheme vector based on the current scheme_name_.
// For efficiency it builds the "kala" array which contains a COLORREF for every byte value.
BOOL CHexEditView::set_colours()
{
    BOOL retval = TRUE;
    std::vector<CScheme>::const_iterator ps;
    ASSERT(theApp.scheme_.size() > 3);

    // First find the scheme
    for (ps = theApp.scheme_.begin(); ps != theApp.scheme_.end(); ++ps)
    {
        if (ps->name_ == scheme_name_)
            break;
    }

    // If the scheme was not found use the standard one matching selected options
    if (ps == theApp.scheme_.end())
    {
        // Current scheme name is not found so set to something OK and return error
        if (display_.char_set == CHARSET_EBCDIC)
            ps = theApp.scheme_.begin()+3;
        else if (display_.char_set == CHARSET_OEM)
            ps = theApp.scheme_.begin()+2;
        else if (display_.char_set == CHARSET_ANSI)
            ps = theApp.scheme_.begin()+1;
        else
            ps = theApp.scheme_.begin();
        scheme_name_ = ps->name_;
        retval = FALSE;
    }

    // Get colours from scheme, where -1 means use default (Automatic) colour
    bg_col_        = ps->bg_col_ == -1       ? ::GetSysColor(COLOR_WINDOW) : ps->bg_col_;
    mark_col_      = ps->mark_col_ == -1     ? RGB(224, 224, 224) : ps->mark_col_;
    hi_col_        = ps->hi_col_ == -1       ? RGB(255, 255, 0) : ps->hi_col_;              // yellow
    bm_col_        = ps->bm_col_ == -1       ? RGB(160, 192, 255) : ps->bm_col_;
    search_col_    = ps->search_col_ == -1   ? RGB(160, 255, 224) : ps->search_col_;
#ifdef CHANGE_TRACKING
    addr_bg_col_   = ps->addr_bg_col_ == -1  ? ::tone_down(::GetSysColor(COLOR_INACTIVEBORDER), bg_col_) : ps->addr_bg_col_;
    // Getting change tracking colour and make a version closer to the background colour in luminance
    // trk_col_ is used to underline replacements, trk_bg_col_ is used as background for insertions
    trk_col_ = ps->trk_col_ == -1 ? RGB(255, 0, 0) : ps->trk_col_;              // red
    trk_bg_col_    = ::tone_down(trk_col_, bg_col_);

    sector_col_    = ps->sector_col_ == -1   ? RGB(255, 160, 128)   : ps->sector_col_;          // pinkish orange
    sector_bg_col_ = ::tone_down(sector_col_, bg_col_);
#else
    addr_bg_col_   = bg_col_;
#endif
    hex_addr_col_  = ps->hex_addr_col_ == -1 ? ::GetSysColor(COLOR_WINDOWTEXT) : ps->hex_addr_col_;
    dec_addr_col_  = ps->dec_addr_col_ == -1 ? ::GetSysColor(COLOR_WINDOWTEXT) : ps->dec_addr_col_;

    if (ps->range_col_.size() > CColourSchemes::INDEX_LAST)
        text_col_ = ps->range_col_[CColourSchemes::INDEX_LAST]; // was back()
    else
        text_col_ = ::GetSysColor(COLOR_WINDOWTEXT);

    int ii;

    // Default colours to background (makes unspecified colours invisible)
    for (ii = 0; ii < 256; ++ii)
        kala[ii] = bg_col_;

    ASSERT(ps->range_val_.size() == ps->range_col_.size());
    for (ii = ps->range_val_.size(); ii > 0; ii--)
    {
        COLORREF colour = (ps->range_col_[ii-1] == -1 ? ::GetSysColor(COLOR_WINDOWTEXT) : ps->range_col_[ii-1]);

        // Set colour for all in this range
        for (range_set<int>::const_iterator rr = ps->range_val_[ii-1].begin(); rr != ps->range_val_[ii-1].end(); ++rr)
        {
            kala[*rr] = colour;
        }
    }

    // Keep data format view colours in sync
    if (pdfv_ != NULL)
    {
        ASSERT_KINDOF(CDataFormatView, pdfv_);
        pdfv_->set_colours();
    }

    return retval;
}

CFont *CHexEditView::SetFont(CFont *ff)
{
    CFont *retval = CScrView::SetFont(ff);

    // Work out size of printer font based on the new screen font
    TEXTMETRIC tm;
    CClientDC dc(this);
    OnPrepareDC(&dc);                   // Get screen map mode/font
    dc.GetTextMetrics(&tm);
    CPoint convert(0, tm.tmHeight);
    dc.LPtoDP(&convert);                // Convert screen font size to device units
    fontsize_.Format("%d", int(convert.y*72.0/dc.GetDeviceCaps(LOGPIXELSX) + 0.5));

    dc.SetMapMode(print_map_mode_);
    dc.DPtoLP(&convert);                // Get printer font size in logical units
    print_lfHeight_ = labs(convert.y);

    return retval;
}

// functor object used in searching hl_set_.range_.
struct segment_compare
{
    bool operator()(const range_set<FILE_ADDRESS>::segment &x, const range_set<FILE_ADDRESS>::segment &y) const
    {
        return x.sfirst < y.sfirst;
    }
};

// These update hints are handled:
// CRemoveHint: removes undo info (without undoing anything) - for when document saved
// CUndoHint: undo changes up to last doc change - called prior to undoing changes
// CHexHint: indicates the document has changed (including as a result of doc undo)
// CBGSearchHint: indicates a background search on the doc has finished

void CHexEditView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
    // Check if this is a change caused by a view
    if (pHint != NULL && pHint->IsKindOf(RUNTIME_CLASS(CRemoveHint)))
    {
        // Remove undo info (as opposed to actually undoing everything as below)
        num_entered_ = num_del_ = num_bs_ = 0;  // Changes now gone from doc undo
        CRemoveHint *prh = dynamic_cast<CRemoveHint *>(pHint);
        std::vector <view_undo, allocator<view_undo> >::iterator pu;
        for (pu = undo_.end(); pu != undo_.begin(); pu--)
            if ((pu-1)->utype == undo_change)
            {
                // Remove everything up to & including the last doc change undo
                ASSERT((pu-1)->index == prh->index);
                undo_.erase(undo_.begin(), pu);
                break;
            }
    }
    else if (pHint != NULL && pHint->IsKindOf(RUNTIME_CLASS(CUndoHint)))
    {
        num_entered_ = num_del_ = num_bs_ = 0;

        // Undo all view changes up to last doc change (no window invalidate)
        CUndoHint *puh = dynamic_cast<CUndoHint *>(pHint);

        // If this is not the view that is undoing
        if (puh->pview != this)
        {
            // Undo all view moves etc up to this change
            ASSERT(undo_.size() > 0);
            while (undo_.size() > 0 && undo_.back().utype != undo_change)
                do_undo();
            ASSERT(undo_.size() > 0);
            ASSERT(undo_.back().index == puh->index);
        }
    }
    else if (pHint != NULL && pHint->IsKindOf(RUNTIME_CLASS(CHexHint)))
    {
        // This hint informs the view that the document has changed with
        // an insertion, deletion or overtype.  This may be as a result of a
        // document undo (if is_undo is true)

        // Get current selection before recalc_display changes row offsets
        FILE_ADDRESS start_addr, end_addr;
        GetSelAddr(start_addr, end_addr);
        int row = 0;
        if (start_addr == end_addr && display_.vert_display)
            row = pos2row(GetCaret());
        int prev_addr_width = addr_width_;

        // Make sure CScrView knows about any size changes
        if (display_.vert_display)
            SetTSize(CSizeAp(-1, ((GetDocument()->length() + offset_)/rowsize_ + 1)*3));  // 3 rows of text
        else
            SetTSize(CSizeAp(-1, (GetDocument()->length() + offset_)/rowsize_ + 1));

        recalc_display();  // file length, hence addr_width_, may have changed

        CHexHint *phh = dynamic_cast<CHexHint *>(pHint);
        ASSERT(phh->address >= 0 && phh->address <= GetDocument()->length());

        // Is this the start of a doc modification?
        // (phh->index == -1 if this a continued modification)
        if (!phh->is_undo && phh->index > -1)
        {
            // New change - save mark, caret + change on undo stack
            // This could be optimized to save mark and caret
            // only if they are moved by the change.
            undo_.push_back(view_undo(undo_setmark, phh->ptoo));
            undo_.back().address = mark_;
            undo_.push_back(view_undo(undo_move, TRUE));
            undo_.back().address = start_addr | (FILE_ADDRESS(row)<<62);
            if (start_addr != end_addr)
            {
                undo_.push_back(view_undo(undo_sel, TRUE));
                undo_.back().address = end_addr;
            }
            undo_.push_back(view_undo(undo_change, TRUE));
            if (phh->pview == NULL || phh->pview == this)
                undo_.back().flag = TRUE;
            else
                undo_.back().flag = FALSE;
#ifndef NDEBUG
            undo_.back().index = phh->index;
#endif
        }

        // Move positions of the caret and the mark if address shifted
        if (!phh->is_undo && (phh->utype == mod_insert || phh->utype == mod_insert_file))
        {
            if (end_addr > phh->address)
            {
                end_addr += phh->len;
                if (start_addr >= phh->address)
                    start_addr += phh->len;
                SetSel(addr2pos(start_addr, row), addr2pos(end_addr, row));
            }

            if (mark_ >= phh->address)
                mark_ += phh->len;
        }
        else if (!phh->is_undo &&
                 (phh->utype == mod_delback || phh->utype == mod_delforw))
        {
            // Check if current selection and the deletion intersect
            if (start_addr < phh->address + phh->len && end_addr > phh->address)
            {
                if (start_addr >= phh->address)     // If sel start within deletion ...
                    start_addr = phh->address;      // ... move it to where chars deleted
                if (end_addr <= phh->address + phh->len) // If sel end within deletion ...
                    end_addr = phh->address;        // ... move it to where chars deleted
                else
                    end_addr -= phh->len;           // past deletion so just move it back
                SetSel(addr2pos(start_addr, row), addr2pos(end_addr, row));
            }
            else if (phh->address + phh->len <= start_addr)
            {
                // Deletion is before selection - just move selection backwards
                SetSel(addr2pos(start_addr - phh->len, row), addr2pos(end_addr - phh->len, row));
            }

            if (mark_ > phh->address + phh->len)
                mark_ -= phh->len;
            else if (mark_ > phh->address)
                mark_ = phh->address;
        }

        // Fix highlights
        if (phh->utype == mod_insert || phh->utype == mod_insert_file)
        {
            range_set<FILE_ADDRESS>::range_t::iterator pp =
                lower_bound(hl_set_.range_.begin(),
                            hl_set_.range_.end(),
                            range_set<FILE_ADDRESS>::segment(phh->address+1, phh->address+1),
                            segment_compare());
            // If there is a highlight before insert posn check if insert is within it
            if (pp != hl_set_.range_.begin())
            {
                pp --;
                // If bytes inserted within highlight move end
                if (pp->slast > phh->address)
                    pp->slast += phh->len;
                ++pp;
            }
            // Move up all the following highlights
            for ( ; pp != hl_set_.range_.end(); ++pp)
            {
                ASSERT(pp->sfirst > phh->address);
                pp->sfirst += phh->len;
                pp->slast += phh->len;
            }
        }
        else if (phh->utype == mod_delback || phh->utype == mod_delforw)
        {
            // Remove highlights for deleted bytes (if any)
            hl_set_.erase_range(phh->address, phh->address+phh->len);
            range_set<FILE_ADDRESS>::range_t::iterator pp =
                lower_bound(hl_set_.range_.begin(),
                            hl_set_.range_.end(),
                            range_set<FILE_ADDRESS>::segment(phh->address, phh->address),
                            segment_compare());
            if (pp != hl_set_.range_.begin() && pp != hl_set_.range_.end())
            {
                range_set<FILE_ADDRESS>::range_t::iterator tmp = pp;
                tmp--;
                // If previous abuts current then join them together
                if (pp->sfirst - phh->len <= tmp->slast)
                {
                    ASSERT(pp->sfirst - phh->len == tmp->slast);
                    tmp->slast = pp->slast - phh->len;
                    // Remove extra list elt (and leave pp pointing to next)
                    tmp = pp;
                    pp++;
                    hl_set_.range_.erase(tmp);
                }
            }
            // Move all the following highlights down
            for ( ; pp != hl_set_.range_.end(); ++pp)
            {
                ASSERT(pp->sfirst > phh->address);
                pp->sfirst -= phh->len;
                pp->slast -= phh->len;
            }
        }

        // Work out the addresses of the first and last line displayed
        CRect rct;
        CRectAp clip_rect;                // rectangle for calcs/clipping

        // Calculate where the display in document
        GetClientRect(&rct);      // First: get client rectangle
        clip_rect = ConvertFromDP(rct);

        // Calculate the address of the first byte of the top row of the
        // display and the first byte of the row just past bottom
        FILE_ADDRESS addr_top = (clip_rect.top/line_height_)*rowsize_ - offset_;
        FILE_ADDRESS addr_bot = (clip_rect.bottom/line_height_ + 1)*rowsize_ - offset_;

        if (addr_width_ != prev_addr_width)
        {
            // Addresses on left side are now different width so redraw everything
            DoInvalidate();
        }
        else if (phh->address >= addr_bot ||
            (phh->address + /*(signed)*/ phh->len < addr_top &&
             (phh->utype == mod_replace || phh->utype == mod_repback)))
        {
            // Do nothing - changes after display OR before but with no address shift
            ; /* null statement */
        }
        else if (phh->address < addr_top + rowsize_ &&
                 phh->utype != mod_replace && phh->utype != mod_repback)
        {
            // Whole display changes, due to address shift
            DoInvalidate();
        }
        else if (phh->utype == mod_replace || phh->utype == mod_repback)
        {
            // Replace starting within display - just invalidate changed area
            invalidate_addr_range(phh->address, phh->address + (phh->len<1?1:phh->len));
        }
        else if (GetDocument()->length() +
                    ((phh->utype == mod_insert || phh->utype == mod_insert_file) ? -phh->len : +phh->len) < addr_bot)
        {
            // Insertion/deletion and previous end was within display
            // Just invalidate from address to bottom of display
            CRectAp inv(0, ((phh->address + offset_)/rowsize_) * line_height_,
                      char_pos(rowsize_), ((addr_bot + offset_)/rowsize_) * line_height_);
            CRect dev = ConvertToDP(inv);
            DoInvalidateRect(&dev);
        }
        else
        {
            // Must be insert/delete starting within the displayed area
            // - invalidate from first changed line to end of display
            invalidate_addr_range(phh->address, addr_bot);
        }

        // Remove doc change undo from undo array & do any assoc undos
        if (phh->is_undo && phh->pview != this)
        {
            ASSERT(undo_.size() > 0);
            if (undo_.size() > 0)
            {
                BOOL ptoo = undo_.back().previous_too;

                undo_.pop_back();       // Change already made to doc

//                while (ptoo)

                // Note changes made to allow for "previous_too" file changes
                while (ptoo && undo_.back().utype != undo_change)
                {
                    if (undo_.size() == 0)
                        break;
                    // Undo anything associated to change to doc
                    ptoo = undo_.back().previous_too;
                    do_undo();
                }
            }
        }
        show_prop();
        show_calc();

		// If mark moved and current search is relative to mark then restart bg search
		if (theApp.align_mark_ && mark_ != GetDocument()->base_addr_)
		{
			GetDocument()->StopSearch();
			GetDocument()->base_addr_ = mark_;
			GetDocument()->StartSearch();
		}
    }
    else if (pHint != NULL && pHint->IsKindOf(RUNTIME_CLASS(CBGSearchHint)))
    {
        CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
        CBGSearchHint *ph = dynamic_cast<CBGSearchHint *>(pHint);
        if (ph->finished_ == -1)
        {
            // Occurrences in area ph->start_ to ph->end_ have been added or removed
            // probably due to bytes being inserted or deleted at caret.

            // Get latest set of search occurrences
            ValidateScroll(GetScroll());

            // Redraw area where occurrences added/removed
            invalidate_addr_range(ph->start_, ph->end_ + search_length_ - 1);
        }
        else if (ph->finished_)
        {
            // Background search finished
            
            // Get the display area occurrences using ValidateScroll
            ValidateScroll(GetScroll());

            // Invalidate any areas where new search string was found
            std::vector<pair<FILE_ADDRESS, FILE_ADDRESS> >::const_iterator pp, pend;
            for (pp = search_pair_.begin(), pend = search_pair_.end(); pp != pend; ++pp)
                invalidate_addr_range(pp->first, pp->second);
        }
        else
        {
            // Remove all displayed search strings (but save a copy in tmp for invalidating)
            std::vector<pair<FILE_ADDRESS, FILE_ADDRESS> > tmp;
            tmp.swap(search_pair_);    // Save search_pair_ in tmp (and make it empty)

            // Invalidate any areas where search string is currently displayed
            std::vector<pair<FILE_ADDRESS, FILE_ADDRESS> >::const_iterator pp, pend;
            for (pp = tmp.begin(), pend = tmp.end(); pp != pend; ++pp)
                invalidate_addr_range(pp->first, pp->second);
        }
    }
	else if (pHint != NULL && pHint->IsKindOf(RUNTIME_CLASS(CDFFDHint)))
	{
		// Nothing required (yet?)
	}
    else if (pHint != NULL && pHint->IsKindOf(RUNTIME_CLASS(CSaveStateHint)))
    {
    }
    else if (pHint != NULL && pHint->IsKindOf(RUNTIME_CLASS(CRestoreStateHint)))
    {
    }
    else if (pHint != NULL && pHint->IsKindOf(RUNTIME_CLASS(CBookmarkHint)))
    {
        CBookmarkHint *pbmh = dynamic_cast<CBookmarkHint *>(pHint);
        invalidate_addr_range(pbmh->addr_, pbmh->addr_+1);
    }
    else if (pHint != NULL && pHint->IsKindOf(RUNTIME_CLASS(CTrackHint)))
    {
#ifdef CHANGE_TRACKING
        if (!display_.hide_replace || !display_.hide_insert || !display_.hide_delete)
        {
            CTrackHint *pth = dynamic_cast<CTrackHint *>(pHint);
            invalidate_addr_range(pth->start_, pth->end_);
        }
#endif
    }
    else
    {
        recalc_display();
        CScrView::OnUpdate(pSender, lHint, pHint);
    }
}

void CHexEditView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
    if (bActivate)
        GetDocument()->BGThreadPriority(THREAD_PRIORITY_LOWEST);
    else
        GetDocument()->BGThreadPriority(THREAD_PRIORITY_IDLE);

    CScrView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

// Checks if the document (file) or view (window) is read only.
// If the document is read only it just displays a message and returns.
// If the view is read only it gives the user the option to turn off RO.
// Returns TRUE if the file/view is read only, FALSE if modifiable.
BOOL CHexEditView::check_ro(const char *desc)
{
    CString ss;
    FILE_ADDRESS start_addr, end_addr;
    GetSelAddr(start_addr, end_addr);

    if (display_.readonly && GetDocument()->read_only())
    {
        ss.Format("This file cannot be modified.\r"
                  "(You can't %s.)", desc);
        ::HMessageBox(ss);
        CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
        aa->mac_error_ = 10;
        return TRUE;
    }
    else if (pdfv_ != NULL && pdfv_->ReadOnly(start_addr, end_addr))
    {
        ss.Format("The selection contains one or\r"
                  "more read-only template fields,\r"
                  "hence you can't %s.", desc);
//        ::HMessageBox(ss, MB_OK|MB_HELP, HID_DFFD_RO);
        ::HMessageBox(ss, MB_OK);
        theApp.mac_error_ = 10;
        return TRUE;
    }
    else if (display_.readonly)
    {
        ss.Format("You can't %s since this window is read only.\r"
                  "Do you want to turn off read only mode?", desc);
        if (::HMessageBox(ss, MB_OKCANCEL) != IDOK)
        {
            CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
            aa->mac_error_ = 10;
            return TRUE;
        }
        else
            allow_mods();
    }
    return FALSE;
}

// Check if we have had read errors on the device and warn the user (but only once)
// Note that this will warn once per view not once per file but we don't care too much.
void CHexEditView::check_error()
{
	if (GetDocument()->HasSectorErrors())
	{
		// turn on sector display so user can see which sector(s) had errors
		display_.borders = 1;

		if (!errors_mentioned_)
		{
			::HMessageBox("Read error(s) were reported by this device",
                          MB_OK|MB_ICONSTOP);
			errors_mentioned_ = true;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CHexEditView drawing

// There are several different coordinate systems used.
//  - device: pixels on the physical device (screen or printer)
//  - logical: units set by windows mapping mode
//             for screen MM_TEXT is used (= device coords) with Y axis down
//             for printer MM_HIMETRIC is used with Y axis up
//  - normalised: this is the same as logical but the sign is changed so
//             that Y axis is always down (and X axis is always right)
//             in order to simplify comparisons etc
//  - document: this is the normalised logical coord system with the origin at
//             the very top of the document.
// Note that different mapping modes are used for screen and printer.
//  MM_TEXT is used for screen otherwise scrolling becomes slightly
//  out of whack resulting in missing or extra lines of pixels.
//  MM_TEXT cannot be used for the printer as it comes out tiny on lasers.
//

void CHexEditView::OnDraw(CDC* pDC)
{
    if (pfont_ == NULL) return;

    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());

    CHexEditDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);

    /* This was to allow print preview to be grey when active printer was
     * monochrome, but somehow it's already OK.
    bool preview =  pDC->GetDeviceCaps(NUMCOLORS) == 2 &&
                    pDC->m_hDC != pDC->m_hAttribDC;
     */

    // Are we in overtype mode and file is empty?
    if (display_.overtype && pDoc->length() == 0)
    {
        // Get the reason that there's no data from the document & display it
        CRect cli;
        GetClientRect(&cli);
        pDC->DPtoLP(&cli);
        if (pDC->IsPrinting())
        {
            if (cli.bottom < 0)
                cli.top = -2 * print_text_height_;
            else
                cli.top = 2 * print_text_height_;
        }
        const char *mm = pDoc->why0();
        pDC->SetTextColor(GetDefaultTextCol());
        pDC->DrawText(mm, -1, &cli, DT_CENTER | DT_TOP | DT_NOPREFIX | DT_SINGLELINE);
		//::TextOutW(pDC->GetSafeHdc(), 0, 0, L"TEST", 4);  // this works!
        return;
    }

    const char *hex;
    if (aa->hex_ucase_)
        hex = "0123456789ABCDEF?";
    else
        hex = "0123456789abcdef?";

    CRectAp doc_rect;                           // Display area relative to whole document
    CRect norm_rect;                            // Display area (norm. logical coords)
    // Note that norm_rect may extend slightly outside the physical view area, for example,
    // if only half a line is visible at the top of the screen.

    // These are the first and last "virtual" addresses of the top and (one past) the end
    // of the addresses within norm_rect.  Note that these are not necessarilly the first
    // and last real addresses.  For example, if offset_ != 0 and at the top of file then
    // first_virt will be -ve.  Similarly the file may not even be as long as last_virt.
    FILE_ADDRESS first_virt, last_virt;
    FILE_ADDRESS first_line;                    // First line that needs displaying
    FILE_ADDRESS last_line;                     // One past last line to display

    FILE_ADDRESS first_addr = 0;                // First address to actually display
    FILE_ADDRESS last_addr = pDoc->length();    // One past last address actually displayed

    FILE_ADDRESS line_inc;                      // 1 or -1 depending on draw dirn (up/down)
    CSize rect_inc;                             // How much to move norm_rect each time
    FILE_ADDRESS start_addr, end_addr;          // Address of current selection
    bool neg_x(false), neg_y(false);            // Does map mode have -ve to left or down
    bool has_focus;                             // Does this window have focus?
    int bitspixel = pDC->GetDeviceCaps(BITSPIXEL);
    if (bitspixel > 24) bitspixel = 24;         // 32 is really only 24 bits of colour
    long num_colours = 1L << (bitspixel*pDC->GetDeviceCaps(PLANES));

//    CBCGDrawManager bcgdm(*pDC);

    int line_height, char_width, char_width_w;  // Text sizes

    ASSERT(offset_ >= 0 && offset_ < rowsize_);
    ASSERT(rowsize_ > 0 && rowsize_ <= max_buf);
    ASSERT(group_by_ > 0);

    if (pDC->IsPrinting())
    {
        // Work out "client" rect of a printer page
        norm_rect.top = norm_rect.left = 0;
        norm_rect.bottom = pDC->GetDeviceCaps(VERTRES);
        norm_rect.right  = pDC->GetDeviceCaps(HORZRES);

        // Convert to logical units but with origin at top left of window
        // Note we can't use ConvertFromDP here as this is for printer not screen
        pDC->DPtoLP(&norm_rect);
        if (norm_rect.right < 0)
        {
            neg_x = true;
            norm_rect.right = -norm_rect.right;
        }
        if (norm_rect.bottom < 0)
        {
            neg_y = true;
            norm_rect.bottom = -norm_rect.bottom;
        }

        // Text-only printer drivers under Win98 return number of text lines not pixels for VERTRES
        if (norm_rect.bottom < norm_rect.right/5)
            norm_rect.bottom *= print_text_height_;

        // Since there is no translation (only scaling) between device and logical coords origin does not change
        ASSERT(norm_rect.top == 0 && norm_rect.left == 0);

        // Work out which part of document to display
        if (display_.vert_display)
            doc_rect = CRectAp(norm_rect) + 
                CSizeAp(-margin_size_.cx, curpage_ * FILE_ADDRESS(lines_per_page_) * print_text_height_*3 - margin_size_.cy);
        else
            doc_rect = CRectAp(norm_rect) + 
                CSizeAp(-margin_size_.cx, curpage_ * FILE_ADDRESS(lines_per_page_) * print_text_height_ - margin_size_.cy);

        line_height = print_text_height_;
        if (display_.vert_display) line_height *= 3;
        char_width = print_text_width_;
        char_width_w = print_text_width_w_;
    }
    else
    {
        has_focus = (GetFocus() == this);
        HideCaret();
        neg_x = negx(); neg_y = negy();

        // Get display rect in logical units but with origin at top left of window
        CRect rct;
        GetClientRect(&rct);
        doc_rect = ConvertFromDP(rct);

        // Display = client rectangle translated to posn in document
//        norm_rect = doc_rect - GetScroll();
        norm_rect.left = doc_rect.left - GetScroll().x;
        norm_rect.right = doc_rect.right - GetScroll().x;
        norm_rect.top = int(doc_rect.top - GetScroll().y);
        norm_rect.bottom = int(doc_rect.bottom - GetScroll().y);

        // Get the current selection so that we can display it in reverse video
        GetSelAddr(start_addr, end_addr);

        line_height = line_height_;
        char_width = text_width_;
        char_width_w = text_width_w_;
    }
    pDC->SetBkMode(TRANSPARENT);

    // Get range of addresses that are visible the in window (overridden for printing below)
    first_virt = (doc_rect.top/line_height) * rowsize_ - offset_;
    last_virt  = (doc_rect.bottom/line_height + 1) * rowsize_ - offset_;

    // Work out which lines could possibly be in the display area
    if (pDC->IsPrinting() && print_sel_)
    {
        GetSelAddr(first_addr, last_addr);

        // Start drawing from start of selection (+ allow for subsequent pages)
        first_line = (first_addr+offset_)/rowsize_ + curpage_*FILE_ADDRESS(lines_per_page_);
        last_line = first_line + lines_per_page_;

        // Also if the selection ends on this page set the end
        if (last_line > (last_addr - 1 + offset_)/rowsize_ + 1)
            last_line = (last_addr - 1 + offset_)/rowsize_ + 1;

        line_inc = 1L;
        rect_inc = CSize(0, line_height);

        first_virt = first_line * rowsize_ - offset_;
        last_virt  = first_virt + lines_per_page_*rowsize_;

        // Things are a bit different if merging duplicate lines
        if (dup_lines_)
        {
            if (curpage_ == 0)
                print_next_line_ = first_line;
            else
                first_line = print_next_line_;
            //  We don't know what the last line will be so set to end of selection
            last_line = (last_addr - 1 + offset_)/rowsize_ + 1;
            last_virt = last_line * rowsize_ - offset_;
        }

        /* Work out where to display the 1st line */
        norm_rect.top += margin_size_.cy;
        norm_rect.bottom = norm_rect.top + line_height;
        norm_rect.left -= doc_rect.left;

        doc_rect.top = first_line * line_height - margin_size_.cy;
        doc_rect.bottom = doc_rect.top + lines_per_page_*print_text_height_; // xxx this right???
    }
    else if (pDC->IsPrinting())
    {
        // Draw just the lines on this page
        first_line = curpage_ * FILE_ADDRESS(lines_per_page_);
        last_line = first_line + lines_per_page_;
        first_addr = max(0, first_line*rowsize_ - offset_);
        last_addr = min(pDoc->length(), last_line*rowsize_ - offset_);

        line_inc = 1L;
        rect_inc = CSize(0, line_height);

        first_virt = first_line * rowsize_ - offset_;
        last_virt  = first_virt + lines_per_page_*rowsize_;

        /* Work out where to display the 1st line */
//      norm_rect.top -= int(doc_rect.top - first_line*line_height);
        norm_rect.top += margin_size_.cy;
        norm_rect.bottom = norm_rect.top + line_height;
//        norm_rect.left +=  margin_size_.cx - doc_rect.left;
        norm_rect.left -= doc_rect.left;
    }
    else if (ScrollUp())
    {
        // Draw from bottom of window up since we're scrolling up (looks better)
        first_line = doc_rect.bottom/line_height;
        last_line = doc_rect.top/line_height - 1;
        line_inc = -1L;
        rect_inc = CSize(0, -line_height);

        /* Work out where to display the 1st line */
        norm_rect.top -= int(doc_rect.top - first_line*line_height);
        norm_rect.bottom = norm_rect.top + line_height;
        norm_rect.left -= doc_rect.left;
    }
    else
    {
        // Draw from top of window down
        first_line = doc_rect.top/line_height;
        last_line = doc_rect.bottom/line_height + 1;
        line_inc = 1L;
        rect_inc = CSize(0, line_height);

        /* Work out where to display the 1st line */
        norm_rect.top -= int(doc_rect.top - first_line*line_height);
        norm_rect.bottom = norm_rect.top + line_height;
        norm_rect.left -= doc_rect.left;
    }

    if (first_addr < first_virt) first_addr = first_virt;
    if (last_addr > last_virt) last_addr = last_virt;

    CPoint pt;   // moved this here to avoid a spurious compiler error

    // Skip background drawing in this case becuase it's too hard.
    // Note that this goto greatly simplifies the tests below.
    if (pDC->IsPrinting() && print_sel_ && dup_lines_)
        goto end_of_background_drawing;

    // Note we draw bookmarks, bad sectors and change-tracking deletions first as
    // they are always drawn from the top of screen (or page) downwards.  This is
    // so the user is less likely to notice the wrong direction.

    if (!display_.hide_bookmarks)
    {
        // Draw bookmarks
        for (std::vector<FILE_ADDRESS>::const_iterator pbm = pDoc->bm_posn_.begin();
             pbm != pDoc->bm_posn_.end(); ++pbm)
        {
            if (*pbm >= first_addr && *pbm <= last_addr)
            {
                CRect mark_rect;

                mark_rect.top = int(((*pbm + offset_)/rowsize_) * line_height - doc_rect.top + 1);
//                mark_rect.bottom = mark_rect.top + line_height - 3;
                mark_rect.bottom = mark_rect.top + line_height - 1;
                if (neg_y)
                {
                    mark_rect.top = -mark_rect.top;
                    mark_rect.bottom = -mark_rect.bottom;
                }

                if (!display_.vert_display && display_.hex_area)
                {
                    mark_rect.left = hex_pos(int((*pbm + offset_)%rowsize_), char_width) - 
                                    doc_rect.left;
//                        mark_rect.right = mark_rect.left + 2*char_width;
                    mark_rect.right = mark_rect.left + 2*char_width + 2;
                    if (neg_x)
                    {
                        mark_rect.left = -mark_rect.left;
                        mark_rect.right = -mark_rect.right;
                    }

                    pDC->FillSolidRect(&mark_rect, bm_col_);
                }

                if (display_.vert_display || display_.char_area)
                {
                    mark_rect.left = char_pos(int((*pbm + offset_)%rowsize_), char_width, char_width_w) - 
                                    doc_rect.left + 1;
//                        mark_rect.right = mark_rect.left + char_width_w - 2;
                    mark_rect.right = mark_rect.left + char_width_w;
                    if (neg_x)
                    {
                        mark_rect.left = -mark_rect.left;
                        mark_rect.right = -mark_rect.right;
                    }
                    pDC->FillSolidRect(&mark_rect, bm_col_);
                }
            }
        }
    }

    // Draw mark if within display area
    // xxx if (mark >= first_addr && mark < last_virst && mark <= last_addr) ???
    if (!pDC->IsPrinting() && mark_ >= first_virt && mark_ < last_virt || 
         pDC->IsPrinting() && mark_ >= first_addr && mark_ < last_addr )
    {
        CRect mark_rect;                // Where mark is drawn in logical coords

        mark_rect.top = int(((mark_ + offset_)/rowsize_) * line_height - doc_rect.top + 1);
//        mark_rect.bottom = mark_rect.top + line_height - 3;
        mark_rect.bottom = mark_rect.top + line_height - 1;
        if (neg_y)
        {
            mark_rect.top = -mark_rect.top;
            mark_rect.bottom = -mark_rect.bottom;
        }

        if (!display_.vert_display && display_.hex_area)
        {
            mark_rect.left = hex_pos(int((mark_ + offset_)%rowsize_), char_width) - 
                             doc_rect.left;
//            mark_rect.right = mark_rect.left + 2*char_width;
            mark_rect.right = mark_rect.left + 2*char_width + 2;
            if (neg_x)
            {
                mark_rect.left = -mark_rect.left;
                mark_rect.right = -mark_rect.right;
            }

            pDC->FillSolidRect(&mark_rect, mark_col_);
        }

        if (display_.vert_display || display_.char_area)
        {
            mark_rect.left = char_pos(int((mark_ + offset_)%rowsize_), char_width, char_width_w) - 
                             doc_rect.left + 1;
//            mark_rect.right = mark_rect.left + char_width_w - 2;
            mark_rect.right = mark_rect.left + char_width_w;
            if (neg_x)
            {
                mark_rect.left = -mark_rect.left;
                mark_rect.right = -mark_rect.right;
            }
            pDC->FillSolidRect(&mark_rect, mark_col_);
        }
    }

#ifdef CHANGE_TRACKING
    // Preread device blocks so we know if there are bad sectors
	if (GetDocument()->IsDevice())
    {
		// As long as we ensure that the CFileNC buffer is at least as big as the display area (the
		// distance from start of top line to end of bottom line) then this should read all
		// sectors to be dipslayed.
        unsigned char cc;
		TRACE("xxx Display size is %ld\n", long(last_addr - first_addr));
        pDoc->GetData(&cc, 1, (first_addr + last_addr)/2);
    }

    CPen *pPen, *pOldPen;
    if (display_.borders)
        pPen = new CPen(PS_SOLID, char_width/12+1, sector_col_);
    else
        pPen = new CPen(PS_SOLID, char_width/6+1, addr_bg_col_);
    pOldPen = pDC->SelectObject(pPen);

    if (!pDC->IsPrinting())
    {
        // This will draw outside margins (and is not really necessary esp. if printing borders is on)
        pt.y = 0;
        pt.x = addr_width_*char_width - char_width/2 - doc_rect.left;
        if (neg_x) pt.x = - pt.x;
        pDC->MoveTo(pt);
        pt.y = 30000;
        if (neg_y) pt.y = -30000;
        pDC->LineTo(pt);
        if (!display_.vert_display && display_.hex_area)
        {
            pt.y = 0;
            pt.x = char_pos(0, char_width) - char_width_w/2 - doc_rect.left;
            if (neg_x) pt.x = - pt.x;
            pDC->MoveTo(pt);
            pt.y = 30000;
            if (neg_y) pt.y = -30000;
            pDC->LineTo(pt);
        }
        if (display_.char_area)
        {
            pt.y = 0;
            pt.x = char_pos(rowsize_ - 1, char_width, char_width_w) + (3*char_width_w)/2 - doc_rect.left;
            if (neg_x) pt.x = - pt.x;
            pDC->MoveTo(pt);
            pt.y = 30000;
            if (neg_y) pt.y = -30000;
            pDC->LineTo(pt);
        }
    }

    if (pDoc->pfile1_ != NULL && display_.borders)
    {
        int seclen = pDoc->GetSectorSize();
        ASSERT(seclen > 0);
        bool prev_bad = first_addr == 0;                  // don't display sector separator above top of file

        for (FILE_ADDRESS sector = (first_addr/seclen)*seclen; sector < last_addr; sector += seclen)
        {
            if (pDoc->HasSectorErrors() && pDoc->SectorError(sector/seclen) != NO_ERROR)
            {
                draw_bg(pDC, doc_rect, neg_x, neg_y,
                        line_height, char_width, char_width_w, sector_bg_col_,
                        max(sector, first_addr),
                        min(sector + seclen, last_addr));
                prev_bad = true;
            }
            else if (!prev_bad && sector >= first_addr && sector < last_addr)
            {
                // Just draw a line above the top of the sector
                if (!display_.vert_display && display_.hex_area)
                {
                    // Hex area
                    pt.y = int(((sector + offset_)/rowsize_) * line_height - doc_rect.top - 1);
                    if (neg_y) pt.y = -pt.y;

                    //pt.x = hex_pos(rowsize_ - 1, char_width) + 2*char_width - doc_rect.left;
                    pt.x = char_pos(0, char_width) - char_width/2 - doc_rect.left;
                    if (neg_x) pt.x = - pt.x;
                    pDC->MoveTo(pt);
                    pt.x = hex_pos(int((sector + offset_)%rowsize_), char_width) - char_width/2 - doc_rect.left;
                    if (neg_x) pt.x = - pt.x;
                    pDC->LineTo(pt);

                    if ((sector + offset_)%rowsize_ != 0)
                    {
                        // Draw on line below and vertical bit too
                        pt.y = int(((sector + offset_)/rowsize_ + 1) * line_height - doc_rect.top - 1);
                        if (neg_y) pt.y = -pt.y;
                        pDC->LineTo(pt);
                        pt.x = hex_pos(0, char_width) - doc_rect.left;
                        if (neg_x) pt.x = - pt.x;
                        pDC->LineTo(pt);
                    }
                }
                if (display_.vert_display || display_.char_area)
                {
                    // Do char area (or stacked mode)
                    pt.y = int(((sector + offset_)/rowsize_) * line_height - doc_rect.top - 1);
                    if (neg_y) pt.y = -pt.y;

                    //pt.x = char_pos(rowsize_ - 1, char_width, char_width_w) + char_width_w - doc_rect.left;
                    pt.x = char_pos(rowsize_ - 1, char_width, char_width_w) + (3*char_width_w)/2 - doc_rect.left;
                    if (neg_x) pt.x = - pt.x;
                    pDC->MoveTo(pt);
                    pt.x = char_pos(int((sector + offset_)%rowsize_), char_width, char_width_w) - doc_rect.left;
                    if (neg_x) pt.x = - pt.x;
                    pDC->LineTo(pt);

                    if ((sector + offset_)%rowsize_ != 0)
                    {
                        // Draw on line below and vertical bit too
                        pt.y = int(((sector + offset_)/rowsize_ + 1) * line_height - doc_rect.top - 1);
                        if (neg_y) pt.y = -pt.y;
                        pDC->LineTo(pt);
                        pt.x = char_pos(0, char_width, char_width_w) - doc_rect.left;
                        if (neg_x) pt.x = - pt.x;
                        pDC->LineTo(pt);
                    }
                    // This fills in a little bit between hex/char areas
                    pt.x = char_pos(0, char_width, char_width_w) - char_width_w/2 - doc_rect.left;
                    if (neg_x) pt.x = - pt.x;
                    pDC->LineTo(pt);
                }
                // Draw a little bit more in the gap between address area and left side
                pt.x = addr_width_*char_width - char_width/2 - doc_rect.left;
                if (neg_x) pt.x = - pt.x;
                pDC->MoveTo(pt);
                pt.x = addr_width_*char_width + char_width/8 - doc_rect.left;  // Extra char_width/8 due to one pixel out on screen (and many on printer)
                if (neg_x) pt.x = - pt.x;
                pDC->LineTo(pt);
            }
            else
            {
                prev_bad = false;  // don't draw border at bottom of previous block fill (just remember this one was OK)
            }
        }
    }
    pDC->SelectObject(pOldPen);
    delete pPen;

    // Draw deletion marks always from top (shouldn't be too visible)
    if (!display_.hide_delete)
    {
        COLORREF prev_col = pDC->SetTextColor(bg_col_);

        std::vector<pair<FILE_ADDRESS, FILE_ADDRESS> > *ppr =
            GetDocument()->Deletions();
        std::vector<pair<FILE_ADDRESS, FILE_ADDRESS> >::const_iterator pp;
        for (pp = ppr->begin(); pp != ppr->end(); ++pp)
        {
            // Check if its before or after the display area
            if (pp->first < first_virt)
                continue;
            else if (pp->first > last_virt)
                break;

            CRect draw_rect;

            draw_rect.top = int(((pp->first + offset_)/rowsize_) * line_height - doc_rect.top);
            draw_rect.bottom = draw_rect.top + line_height;
            if (neg_y)
            {
                draw_rect.top = -draw_rect.top;
                draw_rect.bottom = -draw_rect.bottom;
            }

            if (!display_.vert_display && display_.hex_area)
            {
                draw_rect.left = hex_pos(int((pp->first + offset_)%rowsize_), char_width) - 
                                    char_width - doc_rect.left;
                draw_rect.right = draw_rect.left + char_width;
                if (neg_x)
                {
                    draw_rect.left = -draw_rect.left;
                    draw_rect.right = -draw_rect.right;
                }
                pDC->FillSolidRect(&draw_rect, trk_col_);
                char cc = (pp->second > 9 || !display_.delete_count) ? '*' : '0' + char(pp->second);
                pDC->DrawText(&cc, 1, &draw_rect, DT_CENTER | DT_TOP | DT_NOPREFIX | DT_SINGLELINE);
            }
            if (display_.vert_display || display_.char_area)
            {
                draw_rect.left = char_pos(int((pp->first + offset_)%rowsize_), char_width, char_width_w) - 
                                    doc_rect.left - 2;
                draw_rect.right = draw_rect.left + char_width_w/5+1;
                if (neg_x)
                {
                    draw_rect.left = -draw_rect.left;
                    draw_rect.right = -draw_rect.right;
                }
                pDC->FillSolidRect(&draw_rect, trk_col_);
            }
        }
        pDC->SetTextColor(prev_col);   // restore text colour
    }
    // Now draw other change tracking stuff top down OR bottom up depending on ScrollUp()
    if (!pDC->IsPrinting() && ScrollUp())
    {
        // Draw change tracking from bottom up
        if (!display_.hide_replace)
        {
            std::vector<pair<FILE_ADDRESS, FILE_ADDRESS> > *ppr =
                GetDocument()->Replacements();
            std::vector<pair<FILE_ADDRESS, FILE_ADDRESS> >::reverse_iterator pp;
            for (pp = ppr->rbegin(); pp != ppr->rend(); ++pp)
                if (pp->first < last_virt)
                    break;
            for ( ; pp != ppr->rend(); ++pp)
            {
                if (pp->first + pp->second <= first_virt)
                    break;
                draw_bg(pDC, doc_rect, neg_x, neg_y,
                        line_height, char_width, char_width_w, trk_col_,
                        max(pp->first, first_addr), 
                        min(pp->first + pp->second, last_addr), (pDC->IsPrinting() ? print_text_height_ : text_height_)/8);
            }
        }
        if (!display_.hide_insert)
        {
            std::vector<pair<FILE_ADDRESS, FILE_ADDRESS> > *ppr =
                GetDocument()->Insertions();
            std::vector<pair<FILE_ADDRESS, FILE_ADDRESS> >::reverse_iterator pp;
            for (pp = ppr->rbegin(); pp != ppr->rend(); ++pp)
                if (pp->first < last_virt)
                    break;
            for ( ; pp != ppr->rend(); ++pp)
            {
                if (pp->first +pp->second <= first_virt)
                    break;
                draw_bg(pDC, doc_rect, neg_x, neg_y,
                        line_height, char_width, char_width_w, trk_bg_col_,
                        max(pp->first, first_addr), 
                        min(pp->first + pp->second, last_addr));
            }
        }
    }
    else
    {
        // Draw change tracking from top down
        if (!display_.hide_replace)
        {
            std::vector<pair<FILE_ADDRESS, FILE_ADDRESS> > *ppr =
                GetDocument()->Replacements();
            std::vector<pair<FILE_ADDRESS, FILE_ADDRESS> >::const_iterator pp;
            for (pp = ppr->begin(); pp != ppr->end(); ++pp)
                if (pp->first + pp->second > first_virt)
                    break;
            for ( ; pp != ppr->end(); ++pp)
            {
                if (pp->first > last_virt)
                    break;
                draw_bg(pDC, doc_rect, neg_x, neg_y,
                        line_height, char_width, char_width_w, trk_col_,
                        max(pp->first, first_addr), 
                        min(pp->first + pp->second, last_addr), (pDC->IsPrinting() ? print_text_height_ : text_height_)/8);
            }
        }
        if (!display_.hide_insert)
        {
            std::vector<pair<FILE_ADDRESS, FILE_ADDRESS> > *ppr =
                GetDocument()->Insertions();
            std::vector<pair<FILE_ADDRESS, FILE_ADDRESS> >::const_iterator pp;
            for (pp = ppr->begin(); pp != ppr->end(); ++pp)
                if (pp->first + pp->second > first_virt)
                    break;
            for ( ; pp != ppr->end(); ++pp)
            {
                if (pp->first > last_virt)
                    break;
                draw_bg(pDC, doc_rect, neg_x, neg_y,
                        line_height, char_width, char_width_w, trk_bg_col_,
                        max(pp->first, first_addr), 
                        min(pp->first + pp->second, last_addr));
            }
        }
    }
#endif

    if (!display_.hide_highlight)
    {
        // Draw highlighted areas
        if (!pDC->IsPrinting() && ScrollUp())
        {
            range_set<FILE_ADDRESS>::range_t::reverse_iterator pr;
            for (pr = hl_set_.range_.rbegin(); pr != hl_set_.range_.rend(); ++pr)
                if (pr->sfirst < last_addr)
                    break;
            for ( ; pr != hl_set_.range_.rend(); ++pr)
            {
                if (pr->slast <= first_addr)
                    break;
                draw_bg(pDC, doc_rect, neg_x, neg_y,
                        line_height, char_width, char_width_w, hi_col_,
                        max(pr->sfirst, first_addr), 
                        min(pr->slast, last_addr));
            }
        }
        else
        {
            // Draw highlighted areas
            range_set<FILE_ADDRESS>::range_t::const_iterator pr;
            for (pr = hl_set_.range_.begin(); pr != hl_set_.range_.end(); ++pr)
                if (pr->slast > first_addr)
                    break;
            for ( ; pr != hl_set_.range_.end(); ++pr)
            {
                if (pr->sfirst > last_addr)
                    break;
                draw_bg(pDC, doc_rect, neg_x, neg_y,
                        line_height, char_width, char_width_w, hi_col_,
                        max(pr->sfirst, first_addr), 
                        min(pr->slast, last_addr));
            }
        }
    }

    if (pDC->IsPrinting())
    {
        // Draw search string occurrences
        // Note this goes through all search occurrences (since search_pair_ is
        // calculated for the current window) which may be slow but then so is printing.
        std::vector<FILE_ADDRESS> sf = GetDocument()->SearchAddresses(first_addr-search_length_, last_addr+search_length_);
        std::vector<FILE_ADDRESS>::const_iterator pp;

        for (pp = sf.begin(); pp != sf.end(); ++pp)
        {
            draw_bg(pDC, doc_rect, neg_x, neg_y,
                    line_height, char_width, char_width_w, search_col_,
                    max(*pp, first_addr), 
                    min(*pp + search_length_, last_addr));
        }
    }
    else if (ScrollUp())
    {
        // Draw search string occurrences bottom up
        std::vector<pair<FILE_ADDRESS, FILE_ADDRESS> >::reverse_iterator pp, pend;
        for (pp = search_pair_.rbegin(), pend = search_pair_.rend(); pp != pend; ++pp)
        {
            draw_bg(pDC, doc_rect, neg_x, neg_y,
                    line_height, char_width, char_width_w, search_col_,
                    max(pp->first, first_addr), 
                    min(pp->second, last_addr));
        }
    }
    else
    {
        // Draw search string occurrences from top down
        std::vector<pair<FILE_ADDRESS, FILE_ADDRESS> >::const_iterator pp, pend;
        for (pp = search_pair_.begin(), pend = search_pair_.end(); pp != pend; ++pp)
        {
            draw_bg(pDC, doc_rect, neg_x, neg_y,
                    line_height, char_width, char_width_w, search_col_,
                    max(pp->first, first_addr), 
                    min(pp->second, last_addr));
        }
    }

end_of_background_drawing:

    unsigned char buf[max_buf];              // Holds bytes for current line being displayed
    size_t last_col = 0;                     // Number of bytes in buf to display
    unsigned char prev_buf[max_buf];         // Copy of last buf - used for merging repeated lines
    size_t prev_last_col;                    // Number of bytes in last buf that were displayed
    int repeat_count = 0;                    // Number of consec. duplicate lines found so far

    // Move declarations outside loop (faster?)
    CString ss(' ', 24);                     // Temp string for formatting
    CRect tt;                                // Temp rect
    CRect addr_rect;                         // Where address is drawn

    // This was added for vert_display
    int vert_offset = 0;
    if (display_.vert_display)
    {
        if (pDC->IsPrinting())
            vert_offset = print_text_height_;
        else
            vert_offset = text_height_;
        if (neg_y)
            vert_offset = - vert_offset;
//      vert_offset = (vert_offset*15)/16;  // Leave a gap between rows
    }

    // Note: we use != (line != last_line) since we may be drawing from bottom or top
    for (FILE_ADDRESS line = first_line; line != last_line;
                            line += line_inc, norm_rect += rect_inc)
    {
        // Work out where to display line in logical coords (correct sign)
        tt = norm_rect;
        if (neg_x)
        {
            tt.left = -tt.left;
            tt.right = -tt.right;
        }
        if (neg_y)
        {
            tt.top = -tt.top;
            tt.bottom = -tt.bottom;
        }

        // No display needed if outside display area or past end of doc
        // Note: we don't break when past end since we may be drawing from bottom
        if (!pDC->RectVisible(&tt) || line*rowsize_ - offset_ > pDoc->length())
            continue;

        // Take a copy of the last line output to check for repeated lines
        if (pDC->IsPrinting() && print_sel_ && dup_lines_ && last_col > 0)
            memcpy(prev_buf, buf, last_col);
        prev_last_col = last_col;

        // Get the bytes to display
        size_t ii;                      // Column of first byte

        if (line*rowsize_ - offset_ < first_addr)
        {
            last_col = pDoc->GetData(buf + offset_, rowsize_ - offset_, line*rowsize_) +
                        offset_;
            ii = size_t(first_addr - (line*rowsize_ - offset_));
            ASSERT(int(ii) < rowsize_);
        }
        else
        {
            last_col = pDoc->GetData(buf, rowsize_, line*rowsize_ - offset_);
            ii = 0;
        }

        if (line*rowsize_ - offset_ + last_col - last_addr >= rowsize_)
            last_col = 0;
        else if (line*rowsize_ - offset_ + last_col > last_addr)
            last_col = size_t(last_addr - (line*rowsize_ - offset_));

//        TRACE("xxx line %ld rowsize_ %ld offset_ %ld last_col %ld first_addr %ld last_addr %ld\n",
//              long(line), long(rowsize_), long(offset_), long(last_col), long(first_addr), long(last_addr));

        if (pDC->IsPrinting() && print_sel_ && dup_lines_)
        {
            // Check if the line is the same as the last one
            // But NOT if very first line (line 0 on page 0) since we may not display the whole line
            if ((curpage_ > 0 || line > first_line+1) &&
                last_col == prev_last_col && memcmp(buf, prev_buf, last_col) == 0)
            {
                ++repeat_count;
                norm_rect -= rect_inc;
                continue;
            }

            if (repeat_count > 0)
            {
                // Display how many times the line was repeated
                CString mess;
                if (repeat_count == 1)
                    mess = "Repeated once";
                else
                    mess.Format("Repeated %d more times", repeat_count);
                CRect mess_rect = tt;
                mess_rect.left += hex_pos(0, char_width);
                pDC->DrawText(mess, &mess_rect, DT_TOP | DT_LEFT | DT_NOPREFIX | DT_SINGLELINE);

                // Move the current display line down one
                norm_rect += rect_inc;
                tt = norm_rect;
                if (neg_x)
                {
                    tt.left = -tt.left;
                    tt.right = -tt.right;
                }
                if (neg_y)
                {
                    tt.top = -tt.top;
                    tt.bottom = -tt.bottom;
                }

                // Reset the repeated line counter since this was a different line
                repeat_count = 0;
            }

            // See if we are at the bottom of the page yet
            if (norm_rect.bottom > margin_size_.cy + print_text_height_*lines_per_page_)
            {
                print_next_line_ = line;
                break;
            }
        }

        // Don't draw address if off to the left
        if ((addr_width_ - 1)*char_width + tt.left > 0)
        {
            // Draw the address into the window
            // Note: we need sprintf because CString::Format does not support 64 bit numbers
            char *addr_buf = ss.GetBuffer(24);             // reserve space for 64 bit address
            sprintf(addr_buf, addr_format_, line*rowsize_ - offset_ > first_addr ?
                                            line*rowsize_ - offset_ : first_addr);
            ss.ReleaseBuffer(-1);

//            if (pDC->IsPrinting())
//                pDC->SetTextColor(RGB(0,0,0));      // Draw addresses in black on printer
//            else
            if (display_.dec_addr)
                pDC->SetTextColor(GetDecAddrCol());   // Colour of dec addresses
            else
                pDC->SetTextColor(GetHexAddrCol());   // Colour of hex addresses
            if (aa->nice_addr_)
            {
                if (display_.dec_addr)
                {
                    addr_rect = tt;            // tt with right margin where addresses end

                    addr_rect.right = addr_rect.left + (addr_width_ - 1)*char_width;
                    AddCommas(ss);
                    pDC->DrawText(ss, &addr_rect, DT_TOP | DT_RIGHT | DT_NOPREFIX | DT_SINGLELINE);
                }
                else
                {
                    AddSpaces(ss);
                    pDC->DrawText(ss, &tt, DT_TOP | DT_NOPREFIX | DT_SINGLELINE);
                }
            }
            else
            {
                pDC->DrawText(ss, &tt, DT_TOP | DT_NOPREFIX | DT_SINGLELINE);
            }
        }

        COLORREF current_colour = kala[buf[ii]];
        pDC->SetTextColor(current_colour);
        if (display_.vert_display || display_.hex_area)
        {
            // Keep track of the current colour so we only set it when it changes
            int posx = tt.left + hex_pos(0, char_width);                 // Horiz pos of 1st hex column

            // Display each byte as hex (and char if nec.)
            for (size_t jj = ii ; jj < last_col; ++jj)
            {
                if (display_.vert_display)
                {
                    if (posx + int(jj + 1 + jj/group_by_)*char_width_w < 0)
                        continue;
                    else if (posx + int(jj + jj/group_by_)*char_width_w >= tt.right)
                        break;
                }
                else
                {
                    if (posx + int((jj+1)*3 + jj/group_by_)*char_width < 0)
                        continue;
                    else if (posx + int(jj*3 + jj/group_by_)*char_width >= tt.right)
                        break;
                }

                if (current_colour != kala[buf[jj]])
                {
                    current_colour = kala[buf[jj]];
                    pDC->SetTextColor(current_colour);
                }

                if (display_.vert_display)
                {
                    // Now display the character in the top row
                    if (display_.char_set != CHARSET_EBCDIC)
                    {
                        if ((buf[jj] >= 32 && buf[jj] < 127) ||
                            (display_.char_set != CHARSET_ASCII && buf[jj] >= first_char_ && buf[jj] <= last_char_) )
                        {
                            // Display normal char or graphic char if in font
                            pDC->TextOut(posx + (jj + jj/group_by_)*char_width_w, tt.top, (char *)&buf[jj], 1);
                        }
                        else if (display_.control == 0 || buf[jj] >= 32)
                        {
                            // Display control char and other chars as red '.'
                            pDC->TextOut(posx + (jj + jj/group_by_)*char_width_w, tt.top, ".", 1);
                        }
                        else if (display_.control == 1)
                        {
                            // Display control chars as red uppercase equiv.
                            char cc = buf[jj] + 0x40;
                            pDC->TextOut(posx + (jj + jj/group_by_)*char_width_w, tt.top, &cc, 1);
                        }
                        else if (display_.control == 2)
                        {
                            // Display control chars as C escape code (in red)
                            const char *check = "\a\b\f\n\r\t\v\0";
                            const char *display = "abfnrtv0";
                            const char *pp;
                            if (/*buf[jj] != '\0' && */(pp = strchr(check, buf[jj])) != NULL)
                                pp = display + (pp-check);
                            else
                                pp = ".";
                            pDC->TextOut(posx + (jj + jj/group_by_)*char_width_w, tt.top, pp, 1);
                        }
                    }
                    else
                    {
                        // Display EBCDIC (or red dot if not valid EBCDIC char)
                        if (e2a_tab[buf[jj]] == '\0')
                        {
                            pDC->TextOut(posx + (jj + jj/group_by_)*char_width_w, tt.top, ".", 1);
                        }
                        else
                        {
                            pDC->TextOut(posx + (jj + jj/group_by_)*char_width_w, tt.top, (char *)&e2a_tab[buf[jj]], 1);
                        }
                    }

                    // Display the hex digits below that, one below the other
                    pDC->TextOut(posx + (jj + jj/group_by_)*char_width_w, tt.top + vert_offset,   &hex[(buf[jj]>>4)&0xF], 1);
                    pDC->TextOut(posx + (jj + jj/group_by_)*char_width_w, tt.top + vert_offset*2, &hex[buf[jj]&0xF], 1);
                }
                else
                {
                    char hh[2];

                    // Create hex digits and display them
                    hh[0] = hex[(buf[jj]>>4)&0xF];
                    hh[1] = hex[buf[jj]&0xF];

                    // This actually displays the bytes (in hex)!
                    // Note: removed calcs that were previously encapsulated in hex_pos
                    pDC->TextOut(posx + (jj*3 + jj/group_by_)*char_width, tt.top, hh, 2);
                }
            }
        }

        if (!display_.vert_display && display_.char_area)
        {
            // Keep track of the current colour so we only set it when it changes
            int posc = tt.left + char_pos(0, char_width, char_width_w);  // Horiz pos of 1st char column

            for (size_t kk = ii ; kk < last_col; ++kk)
            {
                if (posc + int(kk+1)*char_width_w < 0)
                    continue;
                else if (posc + int(kk)*char_width_w >= tt.right)
                    break;

                if (current_colour != kala[buf[kk]])
                {
                    current_colour = kala[buf[kk]];
                    pDC->SetTextColor(current_colour);
                }

                // Display byte in char display area (as ASCII, EBCDIC etc)
                if (display_.char_set != CHARSET_EBCDIC)
                {
                    if ((buf[kk] >= 32 && buf[kk] < 127) ||
                        (display_.char_set != CHARSET_ASCII && buf[kk] >= first_char_ && buf[kk] <= last_char_) )
                    {
                        // Display normal char or graphic char if in font
                        pDC->TextOut(posc + kk*char_width_w, tt.top, (char *)&buf[kk], 1);
                    }
                    else if (display_.control == 0 || buf[kk] > 31)
                    {
                        // Display control char and other chars as red '.'
                        pDC->TextOut(posc + kk*char_width_w, tt.top, ".", 1);
                    }
                    else if (display_.control == 1)
                    {
                        // Display control chars as red uppercase equiv.
                        char cc = buf[kk] + 0x40;
                        pDC->TextOut(posc + kk*char_width_w, tt.top, &cc, 1);
                    }
                    else if (display_.control == 2)
                    {
                        // Display control chars as C escape code (in red)
                        const char *check = "\a\b\f\n\r\t\v\0";
                        const char *display = "abfnrtv0";
                        const char *pp;
                        if (/*buf[kk] != '\0' && */(pp = strchr(check, buf[kk])) != NULL)
                            pp = display + (pp-check);
                        else
                            pp = ".";
                        pDC->TextOut(posc + kk*char_width_w, tt.top, pp, 1);
                    }
                }
                else
                {
                    // Display EBCDIC (or red dot if not valid EBCDIC char)
                    if (e2a_tab[buf[kk]] == '\0')
                    {
                        pDC->TextOut(posc + kk*char_width_w, tt.top, ".", 1);
                    }
                    else
                    {
                        pDC->TextOut(posc + kk*char_width_w, tt.top, (char *)&e2a_tab[buf[kk]], 1);
                    }
                }
            }
        }

        // If any part of the line is within the current selection
        if (!pDC->IsPrinting() && start_addr < end_addr &&
            end_addr > line*rowsize_ - offset_ && start_addr < (line+1)*rowsize_ - offset_)
        {
            FILE_ADDRESS start = max(start_addr, line*rowsize_ - offset_);
            FILE_ADDRESS   end = min(end_addr, (line+1)*rowsize_ - offset_);
//            ASSERT(end > start);

            ASSERT(display_.hex_area || display_.char_area);
            if (!display_.vert_display && display_.hex_area)
            {
                CRect rev(norm_rect);
                rev.right = rev.left + hex_pos(int(end - (line*rowsize_ - offset_) - 1)) + 2*text_width_;
                rev.left += hex_pos(int(start - (line*rowsize_ - offset_)));
                if (neg_x)
                {
                    rev.left = -rev.left;
                    rev.right = -rev.right;
                }
                if (neg_y)
                {
                    rev.top = -rev.top;
                    rev.bottom = -rev.bottom;
                }
                if (has_focus && !display_.edit_char || num_colours <= 256)
                    pDC->InvertRect(&rev);  // Full contrast reverse video only if in editing in hex area
                else
                    pDC->PatBlt(rev.left, rev.top,
                                rev.right-rev.left, rev.bottom-rev.top,
                                PATINVERT);
            }

            if (display_.vert_display || display_.char_area)
            {
                // Draw char selection in inverse
                CRect rev(norm_rect);
                rev.right = rev.left + char_pos(int(end - (line*rowsize_ - offset_) - 1)) + text_width_w_;
                rev.left += char_pos(int(start - (line*rowsize_ - offset_)));
                if (neg_x)
                {
                    rev.left = -rev.left;
                    rev.right = -rev.right;
                }
                if (neg_y)
                {
                    rev.top = -rev.top;
                    rev.bottom = -rev.bottom;
                }
                if (num_colours <= 256 || has_focus && (display_.vert_display || display_.edit_char))
                    pDC->InvertRect(&rev);
                else
                    pDC->PatBlt(rev.left, rev.top,
                                rev.right-rev.left, rev.bottom-rev.top,
                                PATINVERT);
            }
        }
        else if (theApp.show_other_ && /* has_focus && */
                 !display_.vert_display &&
                 display_.char_area && display_.hex_area &&  // we can only display in the other area if both exist
                 !pDC->IsPrinting() &&
                 start_addr == end_addr && 
                 start_addr >= line*rowsize_ - offset_ && 
                 start_addr < (line+1)*rowsize_ - offset_)
        {
            // Draw "shadow" cursor in the other area
            FILE_ADDRESS start = max(start_addr, line*rowsize_ - offset_);
            FILE_ADDRESS   end = min(end_addr, (line+1)*rowsize_ - offset_);

            CRect rev(norm_rect);
            if (display_.edit_char)
            {
                ASSERT(display_.char_area);
                rev.right = rev.left + hex_pos(int(end - (line*rowsize_ - offset_))) + 2*text_width_;
                rev.left += hex_pos(int(start - (line*rowsize_ - offset_)));
            }
            else
            {
                ASSERT(display_.hex_area);
                rev.right = rev.left + char_pos(int(end - (line*rowsize_ - offset_))) + text_width_w_;
                rev.left += char_pos(int(start - (line*rowsize_ - offset_)));
            }
            if (neg_x)
            {
                rev.left = -rev.left;
                rev.right = -rev.right;
            }
            if (neg_y)
            {
                rev.top = -rev.top;
                rev.bottom = -rev.bottom;
            }
            pDC->PatBlt(rev.left, rev.top, rev.right-rev.left, rev.bottom-rev.top, PATINVERT);
        }
    } /* for */

    if (pDC->IsPrinting() && print_sel_ && dup_lines_)
    {
        curpage_ = -1;  // signal that there is no more to print

        // Display any residual repeated lines count
        if (repeat_count > 0)
        {
            // Display how many times the line was repeated
            CString mess;
            if (repeat_count == 1)
                mess = "Repeated once";
            else
                mess.Format("Repeated %d more times", repeat_count);
            CRect mess_rect = norm_rect;
            if (neg_x)
            {
                mess_rect.left = -mess_rect.left;
                mess_rect.right = -mess_rect.right;
            }
            if (neg_y)
            {
                mess_rect.top = -mess_rect.top;
                mess_rect.bottom = -mess_rect.bottom;
            }
            mess_rect.left += hex_pos(0, char_width);
            pDC->DrawText(mess, &mess_rect, DT_TOP | DT_LEFT | DT_NOPREFIX | DT_SINGLELINE);
        }
    }

    if (!pDC->IsPrinting())
    {
        ShowCaret();
    }
//    move_dlgs();
}

#define USE_ROP2  1

void CHexEditView::draw_bg(CDC* pDC, const CRectAp &doc_rect, bool neg_x, bool neg_y,
                           int line_height, int char_width, int char_width_w,
                           COLORREF clr, FILE_ADDRESS start_addr, FILE_ADDRESS end_addr,
                           int draw_height /*=-1*/)
{
    if (end_addr < start_addr) return;

#ifdef USE_ROP2
    int saved_rop = pDC->SetROP2(R2_NOTXORPEN);
    CPen pen(PS_SOLID, 0, clr);
    CPen *psaved_pen = pDC->SelectObject(&pen);
    CBrush brush(clr);
    CBrush *psaved_brush = pDC->SelectObject(&brush);
#endif

    FILE_ADDRESS start_line = (start_addr + offset_)/rowsize_;
    FILE_ADDRESS end_line = (end_addr + offset_)/rowsize_;
    int start_in_row = int((start_addr+offset_)%rowsize_);
    int end_in_row = int((end_addr+offset_)%rowsize_);

    CRect rct;

    if (start_line == end_line)
    {
        // Draw the block (all on one line)
        rct.bottom = int((start_line+1) * line_height - doc_rect.top);
        rct.top = rct.bottom - (draw_height > 0 ? draw_height : line_height);
        if (neg_y)
        {
            rct.top = -rct.top;
            rct.bottom = -rct.bottom;
        }

        if (!display_.vert_display && display_.hex_area)
        {
            rct.left = hex_pos(start_in_row, char_width) - doc_rect.left;
            rct.right = hex_pos(end_in_row - 1, char_width) + 2*char_width - doc_rect.left;
            if (neg_x)
            {
                rct.left = -rct.left;
                rct.right = -rct.right;
            }

            if (neg_x && rct.left > rct.right || !neg_x && rct.left < rct.right)
#ifdef USE_ROP2
                pDC->Rectangle(&rct);
#else
                pDC->FillSolidRect(&rct, clr);
#endif
        }

        if (display_.vert_display || display_.char_area)
        {
            // rct.top = start_line * line_height;
            // rct.bottom = rct.top + line_height;
            rct.left = char_pos(start_in_row, char_width, char_width_w) - doc_rect.left;
            rct.right = char_pos(end_in_row - 1, char_width, char_width_w) + char_width_w - doc_rect.left;
            if (neg_x)
            {
                rct.left = -rct.left;
                rct.right = -rct.right;
            }
#ifdef USE_ROP2
            pDC->Rectangle(&rct);
#else
            pDC->FillSolidRect(&rct, clr);
#endif
        }

#ifdef USE_ROP2
        pDC->SetROP2(saved_rop);
        pDC->SelectObject(psaved_pen);
        pDC->SelectObject(psaved_brush);
#endif
        return;  // All on one line so that's it
    }

    // Block extends over (at least) 2 lines so draw the partial lines at each end
    rct.bottom = int((start_line+1) * line_height - doc_rect.top);
    rct.top = rct.bottom - (draw_height > 0 ? draw_height : line_height);
    if (neg_y)
    {
        rct.top = -rct.top;
        rct.bottom = -rct.bottom;
    }
    if (!display_.vert_display && display_.hex_area)
    {
        rct.left = hex_pos(start_in_row, char_width) - doc_rect.left;
        rct.right = hex_pos(rowsize_ - 1, char_width) + 2*char_width - doc_rect.left;
        if (neg_x)
        {
            rct.left = -rct.left;
            rct.right = -rct.right;
        }
        ASSERT(neg_x && rct.left > rct.right || !neg_x && rct.left < rct.right);
#ifdef USE_ROP2
        pDC->Rectangle(&rct);
#else
        pDC->FillSolidRect(&rct, clr);
#endif
    }

    if (display_.vert_display || display_.char_area)
    {
        rct.left = char_pos(start_in_row, char_width, char_width_w) - doc_rect.left;
        rct.right = char_pos(rowsize_ - 1, char_width, char_width_w) + char_width_w - doc_rect.left;
        if (neg_x)
        {
            rct.left = -rct.left;
            rct.right = -rct.right;
        }
#ifdef USE_ROP2
        pDC->Rectangle(&rct);
#else
        pDC->FillSolidRect(&rct, clr);
#endif
    }

    // Last (partial) line
    rct.bottom = int((end_line+1) * line_height - doc_rect.top);
    rct.top = rct.bottom - (draw_height > 0 ? draw_height : line_height);
    if (neg_y)
    {
        rct.top = -rct.top;
        rct.bottom = -rct.bottom;
    }
    if (!display_.vert_display && display_.hex_area)
    {
        rct.left = hex_pos(0, char_width) - doc_rect.left;
        rct.right = hex_pos(end_in_row - 1, char_width) + 2*char_width - doc_rect.left;
        if (neg_x)
        {
            rct.left = -rct.left;
            rct.right = -rct.right;
        }
        if (neg_x && rct.left > rct.right || !neg_x && rct.left < rct.right)
#ifdef USE_ROP2
            pDC->Rectangle(&rct);
#else
            pDC->FillSolidRect(&rct, clr);
#endif
    }

    if (display_.vert_display || display_.char_area)
    {
        rct.left = char_pos(0, char_width, char_width_w) - doc_rect.left;
        rct.right = char_pos(end_in_row - 1, char_width, char_width_w) + char_width_w - doc_rect.left;
        if (neg_x)
        {
            rct.left = -rct.left;
            rct.right = -rct.right;
        }
#ifdef USE_ROP2
        pDC->Rectangle(&rct);
#else
        pDC->FillSolidRect(&rct, clr);
#endif
    }

    // Now draw all the full lines
    if (draw_height > 0)
    {
        // Since we ar not doing a complete fill of the lines (eg underline)
        // we have to do each line of text individually
        for (++start_line; start_line < end_line; ++start_line)
        {
            rct.bottom = int((start_line+1) * line_height - doc_rect.top);
            rct.top = rct.bottom - draw_height;
            if (neg_y)
            {
                rct.top = -rct.top;
                rct.bottom = -rct.bottom;
            }
            if (!display_.vert_display && display_.hex_area)
            {
                rct.left = hex_pos(0, char_width) - doc_rect.left;
                rct.right = hex_pos(rowsize_ - 1, char_width) + 2*char_width - doc_rect.left;
                if (neg_x)
                {
                    rct.left = -rct.left;
                    rct.right = -rct.right;
                }
                ASSERT(neg_x && rct.left > rct.right || !neg_x && rct.left < rct.right);
#ifdef USE_ROP2
                pDC->Rectangle(&rct);
#else
                pDC->FillSolidRect(&rct, clr);
#endif
            }

            if (display_.vert_display || display_.char_area)
            {
                rct.left = char_pos(0, char_width, char_width_w) - doc_rect.left;
                rct.right = char_pos(rowsize_ - 1, char_width, char_width_w) + char_width_w - doc_rect.left;
                if (neg_x)
                {
                    rct.left = -rct.left;
                    rct.right = -rct.right;
                }
#ifdef USE_ROP2
                pDC->Rectangle(&rct);
#else
                pDC->FillSolidRect(&rct, clr);
#endif
            }
        }
    }
    else if (start_line + 1 < end_line)
    {
        // Draw the complete lines as one block
        rct.top = int((start_line + 1) * line_height - doc_rect.top);
        rct.bottom = int(end_line * line_height - doc_rect.top);
        if (neg_y)
        {
            rct.top = -rct.top;
            rct.bottom = -rct.bottom;
        }

        if (!display_.vert_display && display_.hex_area)
        {
            rct.left = hex_pos(0, char_width) - doc_rect.left;
            rct.right = hex_pos(rowsize_ - 1, char_width) + 2*char_width - doc_rect.left;
            if (neg_x)
            {
                rct.left = -rct.left;
                rct.right = -rct.right;
            }
            ASSERT(neg_x && rct.left > rct.right || !neg_x && rct.left < rct.right);
#ifdef USE_ROP2
            pDC->Rectangle(&rct);
#else
            pDC->FillSolidRect(&rct, clr);
#endif
        }

        if (display_.vert_display || display_.char_area)
        {
            rct.left = char_pos(0, char_width, char_width_w) - doc_rect.left;
            rct.right = char_pos(rowsize_ - 1, char_width, char_width_w) + char_width_w - doc_rect.left;
            if (neg_x)
            {
                rct.left = -rct.left;
                rct.right = -rct.right;
            }
#ifdef USE_ROP2
            pDC->Rectangle(&rct);
#else
            pDC->FillSolidRect(&rct, clr);
#endif
        }
    }

#ifdef USE_ROP2
    pDC->SetROP2(saved_rop);
    pDC->SelectObject(psaved_pen);
    pDC->SelectObject(psaved_brush);
#endif
    return;
}

// recalc_display() - recalculates everything to do with the display
// (and redraws it) if anything about how the window is drawn changes.
// This includes font changed, window resized, document changed, display
// options changed (address display, char display, autofit turned on etc).
void CHexEditView::recalc_display()
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());

    // Save info on the current font
    {
        TEXTMETRIC tm;
        CClientDC dc(this);
        OnPrepareDC(&dc);
        dc.GetTextMetrics(&tm);
        text_height_ = tm.tmHeight + tm.tmExternalLeading; // Height of font

        first_char_ = tm.tmFirstChar;                   // 1st valid char of font

        last_char_ = tm.tmLastChar;                     // last valid char of font
// This causes problems when in IBM/OEM mode since these fonts have characters right down to zero
//        if (first_char_ < 32) first_char_ = 32;         // Some fonts return 30 but 30,31 are nothing

// The max char width returned by many fonts is much too big (seems to be bigger than any character in font!?)
//      text_width_ = tm.tmMaxCharWidth;                // width of widest char in font
        CSize size;
        ::GetTextExtentPoint32(dc.m_hDC, "D", 1, &size);
        text_width_ = size.cx;                  // width of "D"
        ::GetTextExtentPoint32(dc.m_hDC, "W", 1, &size);
        text_width_w_ = size.cx;                  // width of "W"

        if (display_.vert_display)
            line_height_ = text_height_ * 3;
        else
            line_height_ = text_height_;
    }

    FILE_ADDRESS length = GetDocument()->length();

    // Work out how much room we need to display addresses on left of window
    if (display_.dec_addr)
    {
        if (length < 10000)
        {
            addr_width_ = 7;
            addr_format_ = "%4I64d:";
        }
        else if (length < 100000)
        {
            addr_width_ = 8;
            addr_format_ = "%5I64d:";
        }
        else if (length < 1000000)
        {
            addr_width_ = 9;
            addr_format_ = "%6I64d:";
        }
        else if (length < 10000000)
        {
            addr_width_ = 11;
            addr_format_ = "%7I64d:";
        }
        else if (length < 100000000)
        {
            addr_width_ = 12;
            addr_format_ = "%8I64d:";
        }
        else if (length < 1000000000)
        {
            addr_width_ = 13;
            addr_format_ = "%9I64d:";
        }
        else if (length < 10000000000)
        {
            addr_width_ = 15;
            addr_format_ = "%10I64d:";
        }
        else if (length < 100000000000)
        {
            addr_width_ = 16;
            addr_format_ = "%11I64d:";
        }
        else if (length < 1000000000000)
        {
            addr_width_ = 17;
            addr_format_ = "%12I64d:";
        }
        else if (length < 10000000000000)
        {
            addr_width_ = 19;
            addr_format_ = "%13I64d:";
        }
        else
        {
            addr_width_ = 20;
            addr_format_ = "%14I64d:";
        }
    }
    else if(aa->hex_ucase_)
    {
        if (length < 0x10000)
        {
            addr_width_ = 6;
            addr_format_ = "%04I64X:";
        }
        else if (length < 0x1000000)
        {
            addr_width_ = 9;
            addr_format_ = "%06I64X:";
        }
        else if (length < 0x100000000)
        {
            addr_width_ = 11;
            addr_format_ = "%08I64X:";
        }
        else if (length < 0x10000000000)
        {
            addr_width_ = 14;
            addr_format_ = "%010I64X:";
        }
        else
        {
            addr_width_ = 16;
            addr_format_ = "%012I64X:";
        }
    }
    else
    {
        if (length < 0x10000)
        {
            addr_width_ = 6;
            addr_format_ = "%04I64x:";
        }
        else if (length < 0x1000000)
        {
            addr_width_ = 9;
            addr_format_ = "%06I64x:";
        }
        else if (length < 0x100000000)
        {
            addr_width_ = 11;
            addr_format_ = "%08I64x:";
        }
        else if (length < 0x10000000000)
        {
            addr_width_ = 14;
            addr_format_ = "%010I64x:";
        }
        else
        {
            addr_width_ = 16;
            addr_format_ = "%012I64x:";
        }
    }

    // Fit columns to window width?
    if (display_.autofit)
    {
        CRect cli;              /* Client rectangle */
        CRectAp rect;           // Client rectangle in norm. coords
        GetClientRect(&cli);
        rect = ConvertFromDP(cli) - GetScroll();
        ASSERT(rect.left == 0 && rect.top == 0);

        // Work out the scroll pos (address of byte in top left of display)
//        CPointAp pp = GetScroll();
//        FILE_ADDRESS topleft = (pp.y/line_height_) * rowsize_;

        // Work out the current address of the caret/selection
//      FILE_ADDRESS address = pos2addr(GetCaret());
        FILE_ADDRESS start_addr, end_addr;
        GetSelAddr(start_addr, end_addr);
        int row = 0;
        if (start_addr == end_addr && display_.vert_display)
            row = pos2row(GetCaret());

        // Work out how many columns we can display across the window
        // NOTE: These calcs are directly related to the calcs in hex_pos and char_pos
        
        // Work out width of display area (total minus address area width)
        int daw = rect.right - addr_width_*text_width_ - text_width_/2 - 1;
        if (display_.vert_display)
        {
            int group_offset = (daw/text_width_w_)%(group_by_ + 1);
            if (group_offset == 0) group_offset = group_by_;
            rowsize_ = ((daw/text_width_w_ - 1)/(group_by_ + 1)) * group_by_ + group_offset;

            // Make sure scroll bars are not shown
            ASSERT(rowsize_ < 5 || rect.right >= char_pos(rowsize_-1)+text_width_w_);
        }
        else if (display_.char_area && display_.hex_area)
        {
            //rowsize_ = (daw - daw/(4*group_by_ + 1))/(3*text_width_ + text_width_w_);
            int sec_len = group_by_*(3*text_width_ + text_width_w_) + text_width_;
            int group_offset = (daw % sec_len)/(3*text_width_ + text_width_w_);
            if (group_offset == group_by_) group_offset = 0;
            rowsize_ = ((daw + text_width_) / sec_len) * group_by_ + group_offset;
            ASSERT(rowsize_ < 5 || rect.right >= char_pos(rowsize_-1)+text_width_w_);
        }
        else if (display_.char_area)
        {
            // This is easy as char area has no grouping
            rowsize_ = daw/text_width_w_;
            ASSERT(rowsize_ < 5 || rect.right >= char_pos(rowsize_-1)+text_width_w_);
        }
        else
        {
            ASSERT(display_.hex_area);
            display_.hex_area = TRUE; // defensive
            //rowsize_ = (daw - daw/(3*group_by_+1))/(3*text_width_);
            int group_offset = (daw/text_width_ - 2)%(3*group_by_ + 1);
            if (group_offset == 3*group_by_) group_offset--;

            rowsize_ = ((daw/text_width_ - 2)/(3*group_by_ + 1))*group_by_ + group_offset/3 + 1;
            ASSERT(rowsize_ < 5 || rect.right >= hex_pos(rowsize_-1)+2*text_width_);
        }

        // Must display at least 4 columns & no more than buffer can hold
        if (rowsize_ < 4)
            rowsize_ = 4;
        else if (rowsize_ > max_buf)
            rowsize_ = max_buf;
        if (real_offset_ >= rowsize_)
            offset_ = rowsize_ - 1;
        else
            offset_ = real_offset_;

        // Set size before setting scroll/caret to avoid them being moved to "valid" pos
        if (display_.vert_display)
            SetTSize(CSizeAp(-1, ((GetDocument()->length() + offset_)/rowsize_ + 1)*3));  // 3 rows of text
        else
            SetTSize(CSizeAp(-1, (GetDocument()->length() + offset_)/rowsize_ + 1));

        // Make sure scroll position is at left side
//        SetScroll(CPointAp(0, topleft/rowsize_ * line_height_));
        SetScroll(CPointAp(0, GetScroll().y));

        // Move the caret/selection to the where the same byte(s) as before
        SetSel(addr2pos(start_addr, row), addr2pos(end_addr, row));
    }
    else
    {
        if (display_.vert_display)
            SetTSize(CSizeAp(-1, ((GetDocument()->length() + offset_)/rowsize_ + 1)*3));  // 3 rows of text
        else
            SetTSize(CSizeAp(-1, (GetDocument()->length() + offset_)/rowsize_ + 1));
    }

    // Make sure we know the width of the display area
    if (display_.vert_display || display_.char_area)
        SetSize(CSize(char_pos(rowsize_-1)+text_width_w_, -1));
    else
    {
        ASSERT(display_.hex_area);
        display_.hex_area = TRUE; // defensive
        SetSize(CSize(hex_pos(rowsize_-1)+2*text_width_, -1));
    }
} /* recalc_display() */

// Return display position given a hex area column number
// int CHexEditView::hex_pos(int column, int width) const;

// Return closest hex area column given x display position
// If inside==TRUE then will return a value from 0 to rowsize_ - 1
// else may return rowsize to indicate being past last column.
int CHexEditView::pos_hex(int pos, BOOL inside) const
{
    int col = pos - addr_width_*text_width_;
    col -= (col/(text_width_*(group_by_*3+1)))*text_width_;
    col = col/(3*text_width_);

    // Make sure col is within valid range
    if (col < 0) col = 0;
    else if (inside && col >= rowsize_) col = rowsize_ - 1;
    else if (col > rowsize_) col = rowsize_;

    return col;
}

// Return display position given a char area column number
// int CHexEditView::char_pos(int column, int width /* = 0 */) const;

// Return closest char area column given display (Y) coord
// If inside==TRUE then will return a value from 0 to rowsize_ - 1
// else may return rowsize_ to indicate being past the last column.
int CHexEditView::pos_char(int pos, BOOL inside) const
{
    int col;
    if (display_.vert_display)
    {
        col = (pos - addr_width_*text_width_)/text_width_w_;
        col -= col/(group_by_+1);
    }
    else if (display_.hex_area)
        col = (pos - addr_width_*text_width_ - rowsize_*3*text_width_ -
                ((rowsize_-1)/group_by_)*text_width_) / text_width_w_;
    else
        col = (pos - addr_width_*text_width_) / text_width_w_;

    // Make sure col is within valid range
    if (col < 0) col = 0;
    else if (inside && col >= rowsize_) col = rowsize_ - 1;
    else if (col > rowsize_) col = rowsize_;

    return col;
}

void CHexEditView::DoInvalidate()
{
    if (((CHexEditApp *)AfxGetApp())->refresh_off_)
        needs_refresh_ = true;
    else
        CScrView::DoInvalidate();
}

void CHexEditView::DoInvalidateRect(LPCRECT lpRect)
{
    if (((CHexEditApp *)AfxGetApp())->refresh_off_)
        needs_refresh_ = true;
    else
        CScrView::DoInvalidateRect(lpRect);
}

void CHexEditView::DoInvalidateRgn(CRgn* pRgn)
{
    if (((CHexEditApp *)AfxGetApp())->refresh_off_)
        needs_refresh_ = true;
    else
        CScrView::DoInvalidateRgn(pRgn);
}

void CHexEditView::DoScrollWindow(int xx, int yy)
{
    if (((CHexEditApp *)AfxGetApp())->refresh_off_)
        needs_refresh_ = true;
    else
        CScrView::DoScrollWindow(xx, yy);
}

void CHexEditView::DoUpdateWindow()
{
    if (!((CHexEditApp *)AfxGetApp())->refresh_off_)
        CScrView::DoUpdateWindow();
}

void CHexEditView::DoHScroll(int total, int page, int pos)
{
    if (((CHexEditApp *)AfxGetApp())->refresh_off_)
    {
        needs_hscroll_ = true;
        h_total_ = total;
        h_page_ = page;
        h_pos_ = pos;
    }
    else
        CScrView::DoHScroll(total, page, pos);
}

void CHexEditView::DoVScroll(int total, int page, int pos)
{
    if (((CHexEditApp *)AfxGetApp())->refresh_off_)
    {
        needs_vscroll_ = true;
        v_total_ = total;
        v_page_ = page;
        v_pos_ = pos;
    }
    else
        CScrView::DoVScroll(total, page, pos);
}

void CHexEditView::DoUpdate()
{
    if (needs_refresh_)
    {
        CScrView::DoInvalidate();
        needs_refresh_ = false;
        CScrView::DoUpdateWindow();
    }

    if (needs_hscroll_)
    {
        CScrView::DoHScroll(h_total_, h_page_, h_pos_);
        needs_hscroll_ = false;
    }

    if (needs_vscroll_)
    {
        CScrView::DoVScroll(v_total_, v_page_, v_pos_);
        needs_vscroll_ = false;
    }
}

void CHexEditView::InvalidateRange(CPointAp start, CPointAp end)
{
    BOOL saved_edit_char = display_.edit_char;          // Saved value of display_.edit_char
    FILE_ADDRESS start_addr, end_addr;          // Range of addresses to invalidate

    // Work out what we are invalidating (hex or char area)
    if (display_.vert_display)
        ; // do nothing since edit_char does not affect pos2addr for vert_display
    else if (!display_.hex_area ||
        (display_.char_area && pos_hex(start.x) == rowsize_ && pos_hex(end.x) == rowsize_))
        display_.edit_char = TRUE;                      // Change display_.edit_char so pos2addr() works
    else
        display_.edit_char = FALSE;

    // Work out range to invalidate (WARNING: this relies on display_.edit_char)
    start_addr = pos2addr(start);
    end_addr = pos2addr(end);

    display_.edit_char = saved_edit_char;               // Restore display_.edit_char

    if (theApp.show_other_)
        ++end_addr;
    else if (start_addr == end_addr)
        return;

    // Note: We need to invalidate an extra char backwards because in the hex
    // display area the white space to the right of the last byte selected is
    // not shown in reverse video.  When the selection is extended towards the
    // end of file (causing InvalidateRange to be called) not only the newly
    // selected bytes need to be invalidated but also the previous one so that
    // the white area after the character is then drawn in reverse video.
    invalidate_addr_range(start_addr-1, end_addr);
}

void CHexEditView::invalidate_addr_range(FILE_ADDRESS start_addr, FILE_ADDRESS end_addr)
{
#if 0 // not needed since handled by DoInvalidateRect calls below
    if (((CHexEditApp *)AfxGetApp())->refresh_off_)
    {
        needs_refresh_ = true;
        return;
    }
#endif

    CRect cli;                          // Client rectangle in device coords
    CRectAp inv;                        // The rectangle to actually invalidate (doc coords)
    CRectAp disp_rect;                  // Rectangle of display in our coords
    GetClientRect(&cli);
    disp_rect = ConvertFromDP(cli);

    // Work out the addresses of the first and (one past) the last byte in display
    FILE_ADDRESS start_disp = (disp_rect.top/line_height_)*rowsize_ - offset_;
    FILE_ADDRESS end_disp = (disp_rect.bottom/line_height_ + 1)*rowsize_ - offset_;

    // Reduce address range to relevant (displayed) area
    if (start_addr < start_disp) start_addr = start_disp;
    if (end_addr > end_disp) end_addr = end_disp;

    if (start_addr >= end_addr)
        return;                         // Nothing to invalidate or all outside display

    // Work out line range that needs invalidating
    FILE_ADDRESS start_line = (start_addr + offset_)/rowsize_;
    FILE_ADDRESS end_line = (end_addr + offset_)/rowsize_;

    // If start and end on the same line just invalidate between them
    if (start_line == end_line)
    {
        ASSERT(display_.hex_area || display_.char_area);
        if (!display_.vert_display && display_.hex_area)
        {
            // Note: (23/11/03) go back an extra text_width_ bytes to allow for 
            // deletion mark (tracking changes) before the byte
            inv = CRectAp(hex_pos(int((start_addr+offset_)%rowsize_))-text_width_, start_line * line_height_,
                          hex_pos(int((end_addr+offset_)%rowsize_)), (start_line + 1) * line_height_);
            CRect dev = ConvertToDP(inv);
            DoInvalidateRect(&dev);
        }
        if (display_.vert_display || display_.char_area)
        {
            // Note: (23/11/03) go back an extra 2 pixels to allow for 
            // deletion mark (tracking changes) before the byte
            inv = CRectAp(char_pos(int((start_addr+offset_)%rowsize_))-2, start_line * line_height_,
                          char_pos(int((end_addr+offset_)%rowsize_)), (start_line + 1) * line_height_);
            CRect dev = ConvertToDP(inv);
            DoInvalidateRect(&dev);
        }
        return;
    }

    // Start line is before end line: invalidate partial lines at each end
    ASSERT(display_.hex_area || display_.char_area);
    if (!display_.vert_display && display_.hex_area)
    {
        // Hex area
        inv = CRectAp(hex_pos(int((start_addr+offset_)%rowsize_))-text_width_, start_line * line_height_,
                      hex_pos(int(rowsize_)), (start_line + 1) * line_height_);
        CRect dev = ConvertToDP(inv);
        DoInvalidateRect(&dev);

        inv = CRectAp(hex_pos(0)-text_width_, end_line * line_height_,
                      hex_pos(int((end_addr+offset_)%rowsize_)), (end_line + 1) * line_height_);
        dev = ConvertToDP(inv);
        DoInvalidateRect(&dev);
    }
    if (display_.vert_display || display_.char_area)
    {
        // Char area or vert_display
        inv = CRectAp(char_pos(int((start_addr+offset_)%rowsize_))-2, start_line * line_height_,
                      char_pos(int(rowsize_)), (start_line + 1) * line_height_);
        CRect dev = ConvertToDP(inv);
        DoInvalidateRect(&dev);

        inv = CRectAp(char_pos(0)-2, end_line * line_height_,
                      char_pos(int((end_addr+offset_)%rowsize_)), (end_line + 1) * line_height_);
        dev = ConvertToDP(inv);
        DoInvalidateRect(&dev);
    }

    // If more than one line between start and end then invalidate that block too
    if (start_line + 1 < end_line)
    {
        ASSERT(display_.hex_area || display_.char_area);
        if (!display_.vert_display && display_.hex_area)
        {
            inv = CRectAp(hex_pos(0)-text_width_, (start_line + 1) * line_height_,
                        hex_pos(rowsize_), (end_line) * line_height_);
            CRect dev = ConvertToDP(inv);
            DoInvalidateRect(&dev);
        }
        if (display_.vert_display || display_.char_area)
        {
            inv = CRectAp(char_pos(0)-2, (start_line + 1) * line_height_,
                        char_pos(rowsize_), (end_line) * line_height_);
            CRect dev = ConvertToDP(inv);
            DoInvalidateRect(&dev);
        }
    }
}

// Override CScrView::ValidateScroll()
void CHexEditView::ValidateScroll(CPointAp &pos, BOOL strict /* =FALSE */)
{
    CScrView::ValidateScroll(pos, strict);

    // Get search occurrences currently in display area whenever we change the scroll posn -
    // this saves checking all addresses (could be millions) in OnDraw
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
    search_pair_.clear();
    if (aa->bg_search_ && aa->pboyer_ != NULL)
    {
        CHexEditDoc *pdoc = GetDocument();
        CRect cli;
        CRectAp rct;
        GetClientRect(&cli);
        rct = ConvertFromDP(cli);
        FILE_ADDRESS start, end;            // range of file addresses within display

        // Get all occurrences that are within the display - this includes those
        // that have an address before the start of display but extend into it.
        start = (pos.y/line_height_)*rowsize_ - offset_     // First addr in display
                - (aa->pboyer_->length() - 1);              // Length of current search string - 1
        if (start < 0) start = 0;                           // Just in case (prob not nec.)
        end = ((pos.y+rct.Height())/line_height_ + 1)*rowsize_ - offset_;

        std::vector<FILE_ADDRESS> sf = pdoc->SearchAddresses(start, end);
        search_length_ = aa->pboyer_->length();
        std::vector<FILE_ADDRESS>::const_iterator pp = sf.begin();
        std::vector<FILE_ADDRESS>::const_iterator pend = sf.end();
        pair<FILE_ADDRESS, FILE_ADDRESS> good_pair;

        if (pp != pend)
        {
            good_pair.first = *pp;
            good_pair.second = *pp + search_length_;
            while (++pp != pend)
            {
                if (*pp >= good_pair.second)
                {
                    search_pair_.push_back(good_pair);
                    good_pair.first = *pp;
                }
                good_pair.second = *pp + search_length_;
            }
            search_pair_.push_back(good_pair);
        }
    }
}

// Override CScrView::ValidateCaret()
void CHexEditView::ValidateCaret(CPointAp &pos, BOOL inside /*=true*/)
{
    // Ensure pos is a valid caret position or move it to the closest such one
    FILE_ADDRESS address = pos2addr(pos, inside);
    if (address < 0)
        address = 0;
    else if (address > GetDocument()->length())
        address = GetDocument()->length();
    if (display_.vert_display)
    {
        pos = addr2pos(address, pos2row(pos));
        // All the following is to avoid problem of dragging a selection within the same "line" but
        // up a "row" and forward a column.  This caused no selection to be drawn and the selection
        // tip to show a negative selection length.
        CPointAp start, end;            // Current selection 
        CPointAp base;                  // Base of current selection (initial point clicked)
        if (GetSel(start, end))
            base = end;
        else
            base = start;
        // If we have a non-zero selection
        if (mouse_down_ && pos != base)
            pos = addr2pos(address, pos2row(base));  // Make end selection row same as start selection row
    }
    else
        pos = addr2pos(address);
}

void CHexEditView::DisplayCaret(int char_width /*= -1*/)
{
    // If no character width given default to width of hex or char area text
    if (char_width == -1 && (display_.edit_char || display_.vert_display))
        char_width = text_width_w_;
    else if (char_width == -1)
        char_width = text_width_;
    CScrView::DisplayCaret(char_width);
    move_dlgs();
}

// Move the modeless dialogs if visible and it/they obscure the caret
void CHexEditView::move_dlgs()
{
    // Get doc size so we know width of visible area
    CSizeAp tt, pp, ll;                 // Size of document total,page,line
    GetSize(tt, pp, ll);

    // Get the selection so we can work out a rectangle bounding the selection
    CPointAp start, end;                // Points of start/end of selection
    GetSel(start, end);

    CRectAp selrect;                    // Rect enclosing selection + a bit
    if (start.y == end.y)               // No selection or all on a single line?
        selrect = CRectAp(start.x - 2*ll.cx, start.y - ll.cy,
                          end.x + 3*ll.cx, end.y + 2*ll.cy);
    else
        selrect = CRectAp(0, start.y - ll.cy,  // multi-line selection
                          tt.cx, end.y + 2*ll.cy);

    // Get display rectangle (also in doc coords)
    CRect cli;
    GetClientRect(&cli);
    CRectAp doc_rect = ConvertFromDP(cli);

    // See if any of the selection is visible (intersect selection & display)
    CRectAp sd_rect;                    // selection that is visible
    if (!sd_rect.IntersectRect(doc_rect, selrect))
        return;                         // Selection not in display

    // Get rect (selection visible in display) in screen (device) coords
    CRect dev_rect = ConvertToDP(sd_rect);
    ClientToScreen(&dev_rect);

    HideCaret();
    // Tell mainframe to move all its dialog bars
    ((CMainFrame *)theApp.m_pMainWnd)->move_bars(dev_rect);
    ShowCaret();
}

// Move scroll or caret position in response to a key press.
// Note that this overrides CScrView::MovePos().
BOOL CHexEditView::MovePos(UINT nChar, UINT nRepCnt,
                         BOOL control_down, BOOL shift_down, BOOL caret_on)
{
    // CScrView::MovePos scrolling behaviour is OK
    if (!caret_on)
        return CScrView::MovePos(nChar, nRepCnt, control_down, shift_down, caret_on);

    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
    if (aa->recording_)
    {
        long vv = nChar;
        if (control_down) vv |= 0x10000;
        if (shift_down)   vv |= 0x20000;
        for (UINT ii = 0; ii < nRepCnt; ++ii)
            aa->SaveToMacro(km_key, vv);
    }

    FILE_ADDRESS start_addr, end_addr;
    BOOL end_base = GetSelAddr(start_addr, end_addr);   // Is selection base at end of selection?
    int row = 0;
    if (start_addr == end_addr && display_.vert_display)
        row = pos2row(GetCaret());
    FILE_ADDRESS new_address;
	CString desc("Cursor key");

    // Start with start of (or end of, if moving forwards) current selection
    if (shift_down)
    {
        // Work out which end of selection is being extended
        if (end_base)
            new_address = start_addr;
        else
            new_address = end_addr;
        ++shift_moves_;
    }
    else if (start_addr == end_addr )
        new_address = start_addr;                       // No current selection
    else if (nChar == VK_DOWN || nChar == VK_NEXT)
        new_address = end_addr;                         // Move from char after selection
    else if (nChar == VK_RIGHT || nChar == VK_END)
        new_address = end_addr - 1;                     // Move from last char of selection
    else
        new_address = start_addr;                       // Move from start of selection

    CSizeAp tt, pp, ll;                   // Size of document total,page,line

    switch (nChar)
    {
    case VK_LEFT:
        if (control_down)
        {
			desc = "Ctrl + Left Arrow";
            // Work out how many groups there are to start of file
            long gpr = (rowsize_ - 1)/group_by_ + 1;    // groups per row
            FILE_ADDRESS groups = ((new_address+offset_)/rowsize_) * gpr +
                          ((new_address+offset_)%rowsize_ + group_by_ - 1)/group_by_;
            // Calculate the group to move to and address of 1st byte
            groups -= nRepCnt;
            new_address = (groups/gpr) * rowsize_ - offset_ + (groups%gpr) * group_by_;
        }
        else
		{
			desc = "Left Arrow";
            new_address -= nRepCnt;
		}
        break;
    case VK_RIGHT:
        if (control_down)
        {
			desc = "Ctrl + Right Arrow";
            // First work out how many groups there are to start of file
            long gpr = (rowsize_ - 1)/group_by_ + 1;    // groups per row
            FILE_ADDRESS groups = ((new_address+offset_)/rowsize_) * gpr +
                          ((new_address+offset_)%rowsize_)/group_by_;
            // Calculate the group to move to
            groups += nRepCnt;
            new_address = (groups/gpr) * rowsize_ - offset_ + (groups%gpr) * group_by_;
        }
        else
		{
			desc = "Right Arrow";
            new_address += nRepCnt;
		}
        break;
    case VK_UP:
		desc = "Up Arrow";
        if (display_.vert_display && !shift_down)
        {
            new_address -= rowsize_ * ((2 - row + nRepCnt)/3);
            row = (3333 + row - nRepCnt)%3;   // Add a large number div. by 3 to make sure % operand is +ve
        }
        else
            new_address -= rowsize_ * nRepCnt;
        break;
    case VK_DOWN:
		desc = "Down Arrow";
        if (display_.vert_display && !shift_down)
        {
            new_address += rowsize_ * ((row + nRepCnt)/3);
            row = (row + nRepCnt)%3;
        }
        else
            new_address += rowsize_ * nRepCnt;
        break;
    case VK_HOME:
        if (control_down)
		{
			desc = "Ctrl + Home key ";  // space at end means significant nav pt
            new_address = 0;
		}
        else
		{
			desc = "Home key";
            new_address = ((new_address+offset_)/rowsize_) * rowsize_ - offset_;
		}
        break;
    case VK_END:
        if (control_down)
		{
			desc = "Ctrl + End key ";  // space at end means significant nav pt
            new_address = GetDocument()->length();
		}
        else
		{
			desc = "End key";
            new_address = ((new_address+offset_)/rowsize_ + 1) * rowsize_ - offset_ - 
                                (shift_down ? 0 : 1);
		}
        break;
    case VK_PRIOR:
		desc = "Page Up";
        GetSize(tt, pp, ll);
        new_address -= rowsize_ * (pp.cy/line_height_) * nRepCnt;
        break;
    case VK_NEXT:
		desc = "Page Down";
        GetSize(tt, pp, ll);
        new_address += rowsize_ * (pp.cy/line_height_) * nRepCnt;
        break;
    default:
        return CScrView::MovePos(nChar, nRepCnt, control_down, shift_down, caret_on);
    }

    if (new_address < 0)
    {
        new_address = 0;
        row = 0;
        aa->mac_error_ = 2;
    }
    else if (new_address > GetDocument()->length())
    {
        new_address = GetDocument()->length();
        if (display_.vert_display && !shift_down)
            row = 2;
        aa->mac_error_ = 2;
    }

    // Scroll addresses into view if moved to left column of hex area or 
    // left column of char area when no hex area displayed
    if ((new_address + offset_) % rowsize_ == 0 && (display_.vert_display || !display_.edit_char || !display_.hex_area))
        SetScroll(CPointAp(0,-1));

    if (shift_down && end_base)
    {
        MoveWithDesc("Shift + " + desc, end_addr, new_address);

        // Handle this when shift key released now (in OnKeyUp)
//        if (aa->highlight_)
//            add_highlight(new_address, end_addr, TRUE);
    }
    else if (shift_down)
    {
        MoveWithDesc("Shift + " + desc, start_addr, new_address);

        // Handle this when shift key released now (in OnKeyUp)
//        if (aa->highlight_)
//            add_highlight(start_addr, new_address, TRUE);
    }
    else
        MoveWithDesc(desc, new_address, -1, -1, -1, FALSE, FALSE, row);

    return TRUE;                // Indicate that keystroke used
}

// Move the caret to new position (or as close as possible).
// Returns the new position which may be diff to the requested position if
// the requested position was invalid (past EOF).
// Note: Does not update address in tool bar combo (use show_pos(GetPos()),
//       or make sure the caret within display, and does not save undo info.
FILE_ADDRESS CHexEditView::GoAddress(FILE_ADDRESS start_addr, FILE_ADDRESS end_addr /*=-1*/)
{
    // Get row from top 2 bits of start_address (vert_display mode)
    int row = int(start_addr>>62) & 0x3;
    start_addr &= 0x3fffFFFFffffFFFF;

    if (end_addr < 0 || end_addr > GetDocument()->length())
        end_addr = start_addr;
    if (start_addr < 0 || start_addr > GetDocument()->length())
        start_addr = end_addr = GetDocument()->length();

    ASSERT(row == 0 || (row < 3 && display_.vert_display && start_addr == end_addr));
    SetSel(addr2pos(start_addr, row), addr2pos(end_addr, row));

    return start_addr;
}

// Move the caret to new position updating everything:
//  - saves previous position in undo array (if caret is actually moved)
//  - updates the display address in the tool bar address edit controls
//  - Makes sure the caret is visible within display
//  - astart/aend = new selection (if aend = -1 or aend=astart then just sets caret)
//  - pstart/pend = previous selection to be saved in undo list (if they are 
//    -1 it uses the current selection/caret)
//  - ptoo = when undo info saved combine it with previous operation (used if the move is part of a larger operation)
//  - no_dffd = don't sync DFFD view (if any) even in sync mode (avoids inf. mutually recursive calls)
//  - row = stacked mode row (0 to 2) - only important when setting caret not for block selection (includes all rows)
//  - desc = describes why a move was made (eg Bookmark, Search) so that info can be given in nav point list
void CHexEditView::MoveToAddress(FILE_ADDRESS astart, FILE_ADDRESS aend /*=-1*/,
                                 FILE_ADDRESS pstart /*=-1*/, FILE_ADDRESS pend /*=-1*/,
                                 BOOL ptoo /*=FALSE*/, BOOL no_dffd /*=FALSE*/,
                                 int row /*=0*/, LPCTSTR desc /*=NULL*/)
{
    ASSERT((astart & ~0x3fffFFFFffffFFFF) == 0); // Make sure top 2 bits not on
    ASSERT(pstart == -1 || (pstart & ~0x3fffFFFFffffFFFF) == 0);

    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing since caret moved

    if (astart < 0 || astart > GetDocument()->length())
    {
        CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
        char buf[128];
        sprintf(buf, "Attempt to jump to invalid address %I64d\r"
                  "Jump to EOF instead and continue?", __int64(astart));
        if (::HMessageBox(buf, MB_OKCANCEL) != IDOK)
        {
            aa->mac_error_ = 10;
            return;
        }
        else
            aa->mac_error_ = 1;

        astart = GetDocument()->length();
    }
    if (aend < 0 || aend > GetDocument()->length())
        aend = astart;
    ASSERT(pstart >= -1 && pstart <= GetDocument()->length());
    ASSERT(pend >= pstart && pend <= GetDocument()->length());

    int prow = 0;   // Row of cursor if vert_display mode
    if (pstart < 0 || pstart > GetDocument()->length() ||
        pend < pstart || pend > GetDocument()->length())
    {
        GetSelAddr(pstart, pend);
        if (pstart == pend && display_.vert_display)
            prow = pos2row(GetCaret());
    }

    // Is the caret/selection now in a different position
    if (astart != pstart || aend != pend || row != prow)
    {
        // Move the caret/selection (THIS IS THE IMPORTANT BIT)
        SetSel(addr2pos(astart, row), addr2pos(aend, row), true);

        // Now check if we are just reversing an operation and we should just remove previous undo
        if (theApp.intelligent_undo_ &&         // intelligent undo is turned on
            astart == aend &&                   // not a selection
            undo_.size() > 0 &&                 // there is an undo op on the stack
            undo_.back().utype == undo_move &&  // previous op was a move
            !undo_.back().previous_too &&       // and not part of another operation
            undo_.back().address == (astart | (FILE_ADDRESS(row)<<62)))  // and same address/row
        {
            undo_.pop_back();
        }
        else
        {
            undo_.push_back(view_undo(undo_move, ptoo));  // Save move in undo array
            undo_.back().address = pstart | (FILE_ADDRESS(prow)<<62);
            if (pstart != pend)
            {
                undo_.push_back(view_undo(undo_sel, ptoo)); // Save selection end in undo array
                undo_.back().address = pend;
            }
        }

		nav_save(astart, aend, desc);

        show_prop();                            // Update prop modeless dlg
        show_calc();
        show_pos(-1, no_dffd);                  // Update tool bar
    }

    DisplayCaret();                             // Make sure caret is in the display
}

void CHexEditView::nav_save(FILE_ADDRESS astart, FILE_ADDRESS aend, LPCTSTR desc)
{
	if (theApp.navman_.InMove())
		return;

	// Check if we have moved enough to store a nav pt
	bool save_nav = false;      // Is this a significant enough move to record in nav stack?
	bool significant = desc != NULL && desc[strlen(desc)-1] == ' ';  // Important nav pts have space at end of desc

	// The number of rows to move before adding a new nav pt (for significant and insignificant events)
	FILE_ADDRESS significant_rows = 2;
	FILE_ADDRESS insignificant_rows = win_size_.cy / line_height_ / 2;

	// Note: The below checks if the caret has moved more than half a page AND the display has
	// scrolled.  BUT if desc is not NULL (a description was supplied) then this is some sort of
	// significant event (search, jump to bookmark etc) so keep even if just moved > 2 lines.

	// Check if the start of selection has moved significantly
	FILE_ADDRESS curr_line = (astart + offset_)/rowsize_;
	FILE_ADDRESS prev_line = (nav_start_ + offset_)/rowsize_;
	if (!significant && mac_abs(curr_line - prev_line) > insignificant_rows ||
		 significant && mac_abs(curr_line - prev_line) > significant_rows)
	{
		save_nav = true;
	}

	// Now do the same test on selection end address
	curr_line = (aend + offset_)/rowsize_;
	prev_line = (nav_end_ + offset_)/rowsize_;
	if (!significant && mac_abs(curr_line - prev_line) > insignificant_rows ||
		 significant && mac_abs(curr_line - prev_line) > significant_rows)
	{
		save_nav = true;
	}

	// If we haven't scrolled vertically yet don't add nav point (unless significant)
	if (!significant && nav_scroll_ == (GetScroll().y/line_height_)*rowsize_ - offset_)
		save_nav = false;

	if (save_nav)
	{
		nav_start_ = astart;
		nav_end_ = aend;
		nav_scroll_ = (GetScroll().y/line_height_)*rowsize_ - offset_;

		// If no descripition given work out something from the file
		if (desc == NULL)
			desc = "Move";

		// Save nav pt in the stack
		theApp.navman_.Add(desc, get_info(), this, nav_start_, nav_end_, nav_scroll_);
	}
}


// Get a bit of the file (as hex or text) to save with a nav point
CString CHexEditView::get_info()
{
	FILE_ADDRESS start = nav_start_;
	if (start >= GetDocument()->length())
	{
		return CString("End\nof\r\nfile");
	}

	if (display_.vert_display && pos2row(GetCaret()) == 0 || display_.edit_char)
	{
		// Take a sample of the file as text
		char buf[36];
		char *pp = buf;
		FILE_ADDRESS aa, end;
		unsigned char cc = '\0';
		end = start + 30;
		if (end > GetDocument()->length())
			end = GetDocument()->length();
		// Just use the selection if it is short enough
		if (start != nav_end_ && nav_end_ < end)
			end = nav_end_;
		for (aa = start; aa < end; ++aa)
		{
			GetDocument()->GetData(&cc, 1, aa);
			if (display_.char_set != CHARSET_EBCDIC)
			{
				if (isprint(cc))
					*pp++ = cc;
				else
					goto hex_info;
			}
			else
			{
				if (e2a_tab[cc] != '\0')
					*pp++ = e2a_tab[cc];
				else
					goto hex_info;
			}
		}
		*pp = '\0';
		if (aa < GetDocument()->length())
			strcat(pp, "...");
		return CString(buf);
	}

hex_info:
	{
		// Take a sample of the file as hex
		char buf[36];
		char *pp = buf;
		FILE_ADDRESS aa, end;
		unsigned char cc = '\0';
		end = start + 10;
		if (end > GetDocument()->length())
			end = GetDocument()->length();
		// Just use the selection if it is short enough
		if (start != nav_end_ && nav_end_ < end)
			end = nav_end_;
		for (aa = start; aa < end; ++aa)
		{
			GetDocument()->GetData(&cc, 1, aa);
			if (theApp.hex_ucase_)
				sprintf(pp, "%2.2X ", cc);
			else
				sprintf(pp, "%2.2x ", cc);
			pp += 3;
		}
		if (aa < GetDocument()->length())
			strcat(pp, "...");
		return CString(buf);
	}
}

#if 0
// This saves a move in the undo array for the caret position when focus
// was last lost.  This is used by the tool bar address edit controls to
// add to the undo list when they move the address of the caret.
void CHexEditView::SaveMove()
{
    undo_.push_back(view_undo(undo_move));
    undo_.back().address = saved_start_ | (FILE_ADDRESS(saved_row_)<<62);
    if (saved_start_ != saved_end_)
    {
        // Save the previous selection
        undo_.push_back(view_undo(undo_sel));
        undo_.back().address = saved_end_;
    }
}
#endif

// Convert an address to a display position
CPointAp CHexEditView::addr2pos(FILE_ADDRESS address, int row /*=0*/) const
{
    ASSERT(row == 0 || (display_.vert_display && row < 3));
    address += offset_;
    if (display_.vert_display)
        return CPointAp(char_pos(int(address%rowsize_)), (address/rowsize_) * line_height_ + row * text_height_);
    else if (display_.edit_char)
        return CPointAp(char_pos(int(address%rowsize_)), address/rowsize_ * line_height_);
    else if ((num_entered_ % 2) == 0)
        return CPointAp(hex_pos(int(address%rowsize_)), address/rowsize_ * line_height_);
    else
        return CPointAp(hex_pos(int(address%rowsize_)) + 2*text_width_,
                      address/rowsize_ * line_height_);
}

FILE_ADDRESS CHexEditView::pos2addr(CPointAp pos, BOOL inside /*=true*/) const
{
    FILE_ADDRESS address;
    address = (pos.y/line_height_)*rowsize_ - offset_;
    if (display_.vert_display || display_.edit_char)
        address += pos_char(pos.x, inside);
    else
        address += pos_hex(pos.x + text_width_/2, inside);
    return address;
}

// Given a point in doc coords returns which of the 3 rows of the line the point is in
// Note that in vert_display mode row 0 = char, row 1 = top nybble, row 2 = bottom nybble
int CHexEditView::pos2row(CPointAp pos)
{
    ASSERT(display_.vert_display);
    ASSERT((pos.y%line_height_)/text_height_ < 3);
    return int((pos.y%line_height_)/text_height_);
}

// Update property display if visible
// address = address of byte(s) to show properties of
//         = -1 to use the current cursor address
//         = -2 to update even when refresh off (using current cursor address)
void CHexEditView::show_prop(FILE_ADDRESS address /*=-1*/)
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
    if (address != -2 && aa->refresh_off_) return;

    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();
    if (address < 0) address = GetPos();
    mm->m_wndProp.m_pSheet->Update(this, address);
}

void CHexEditView::show_calc()
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();
    if (!aa->refresh_off_ && mm->m_wndCalc.IsVisible())
        mm->m_wndCalc.update_controls();
}

// Update hex and decimal address tools in edit bar
// address = address to show in the address tools
//         = -1 to use the current cursor address
//         = -2 to update even when refresh off (using current cursor address)
void CHexEditView::show_pos(FILE_ADDRESS address /*=-1*/, BOOL no_dffd /*=FALSE*/)
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
    if (address != -2 && aa->refresh_off_) return;

    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();
    CString ss;         // String to hold text of edit box

    if (address < 0) address = GetPos();

#if 0 // BCG (this is handled by calls to AfxGetApp()->OnIdle(0) in searches/compares)
    // Set the hex address edit box
    if (aa->hex_ucase_)
        ss.Format("%lX", address);
    else
        ss.Format("%lx", address);
    mm->hec_hex_addr_.SetWindowText(ss);
    mm->hec_hex_addr_.add_spaces();
    mm->hec_hex_addr_.UpdateWindow();           // Force immed. redraw

    // Set the decimal address edit box 
    ss.Format("%lu", address);
    mm->dec_dec_addr_.SetWindowText(ss);
    mm->dec_dec_addr_.add_commas();
    mm->dec_dec_addr_.UpdateWindow();           // Force immed. redraw
#else
//    TRACE1("---- Address set to %ld\n", long(address));
    ((CMainFrame *)AfxGetMainWnd())->SetAddress(address);  // for ON_UPDATE_COMMAND_UI to fix displayed addresses
#endif
    // Move to correspoding element in DFFD view
    if (pdfv_ != NULL && !no_dffd && display_.auto_sync)
        pdfv_->SelectAt(address);
}

/////////////////////////////////////////////////////////////////////////////
// CHexEditView diagnostics

#ifdef _DEBUG
void CHexEditView::AssertValid() const
{
        CScrView::AssertValid();
}

void CHexEditView::Dump(CDumpContext& dc) const
{
    dc << "\nrowsize_   = " << rowsize_;
    dc << "\ntext_height_ = " << text_height_;
    dc << "\ntext_width_  = " << text_width_;
    dc << "\nmark_      = " << long(mark_);
    dc << "  mark_char_ = " << display_.mark_char;
    dc << "\nedit_char_ = " << display_.edit_char;
    dc << "\novertype_  = " << display_.overtype;
    dc << "\nreadonly_  = " << display_.readonly;
    dc << "\nsaved_start_/end_ = " << long(saved_start_) << " " << long(saved_end_);

    std::vector <view_undo, allocator<view_undo> >::const_iterator pu;
    for (pu = undo_.begin(); pu != undo_.end(); ++pu)
    {
        dc << "\nutype = " << (*pu).utype;
        switch (pu->utype)
        {
        case undo_move:
            dc << " MOVE from " << long((*pu).address);
            break;
        case undo_sel:
            dc << " SELECTION to " << long((*pu).address);
            break;
        case undo_change:
            dc << " DOC CHANGE this view = " << (*pu).flag;
            dc << " index = " << (*pu).index;
            break;
        case undo_font:
            dc << " FONT CHANGE ";
            break;
        case undo_setmark:
            dc << " MARK SET address = " << long((*pu).address);
            break;
        case undo_rowsize:
            dc << " ROW SIZE = " << (*pu).rowsize;
            break;
        case undo_group_by:
            dc << " GROUP BY = " << (*pu).rowsize;
            break;
        case undo_offset:
            dc << " OFFSET   = " << (*pu).rowsize;
            break;
        case undo_unknown:
            dc << " UNKNOWN";
            break;
        default:
            dc << " NOT RECOGNISED";
            break;
        }
      }

    CScrView::Dump(dc);
}

CHexEditDoc* CHexEditView::GetDocument() // non-debug version is inline
{
        ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CHexEditDoc)));
        return (CHexEditDoc*)m_pDocument;
}
#endif //_DEBUG

// Get child frame window of this view.  This may be the parent if there are no splitters,
// or it may be a more distant ancestor if the immediate parent is a splitter.
// This could possibly return NULL but I'm not sure why.
CChildFrame *CHexEditView::GetFrame() const
{
    CWnd *pp = GetParent();
    while (pp != NULL && !pp->IsKindOf(RUNTIME_CLASS(CChildFrame)))
        pp = pp->GetParent();
    ASSERT_KINDOF(CChildFrame, pp);

    return (CChildFrame *)pp;
}

/////////////////////////////////////////////////////////////////////////////
// CHexEditView message handlers

BOOL CHexEditView::OnEraseBkgnd(CDC* pDC)
{
    if (bg_col_ == -1) return CScrView::OnEraseBkgnd(pDC);

    CRect rct;
    GetClientRect(rct);

    // Fill background with bg_col_
    CBrush backBrush;
    backBrush.CreateSolidBrush(bg_col_);
    backBrush.UnrealizeObject();
    pDC->FillRect(rct, &backBrush);

    // Get rect for address area
    rct.right = hex_pos(0) - GetScroll().x - text_width_/2;

    // If address area is visible and address background is different to normal background ...
    if (rct.right > rct.left && addr_bg_col_ != bg_col_)
    {
        // Draw address background too
        CBrush addrBrush;
        addrBrush.CreateSolidBrush(addr_bg_col_);
        addrBrush.UnrealizeObject();
        pDC->FillRect(rct, &addrBrush);
    }

    return TRUE;
}

void CHexEditView::OnSize(UINT nType, int cx, int cy) 
{
    if (cx == 0 && cy == 0)
    {
        // Not a "real" resize event
        CScrView::OnSize(nType, cx, cy);
        return;
    }
    num_entered_ = num_del_ = num_bs_ = 0;      // Can't be editing while mousing

    BOOL caret_displayed = CaretDisplayed(win_size_);

    if (text_height_ > 0)
    {
        // New code that keeps caret at same line in autofit mode when resizing
        FILE_ADDRESS keep_address;                  // Address we want to keep in the display
        int keep_row = 0;                           // Row within line if vert_display mode
        CPoint keep_point;                          // Point in client rect where we want to keep the point

        // Use caret as address to keep on same display line else use middle address of window
        if (caret_displayed)
        {
            // keep_address = GetPos();
            FILE_ADDRESS end_addr;
            GetSelAddr(keep_address, end_addr);
            if (keep_address == end_addr && display_.vert_display)
                keep_row = pos2row(GetCaret());
        }
        else
        {
            // Get rectangle of display area within document
            CRectAp doc_rect = CRectAp(GetScroll(), win_size_);

            // Get average address of top and bottom of window
            FILE_ADDRESS first_virt = (doc_rect.top/line_height_) * rowsize_ - offset_;
            FILE_ADDRESS last_virt = (doc_rect.bottom/line_height_ + 1) * rowsize_ - offset_;
            keep_address = (first_virt + last_virt)/2;

            // Make sure address is within the file.  Note that top can be before the
            // start of file (if offset_ > 0) and bottom can be past the end of file.
            if (keep_address < 0) keep_address = 0;
            else if (keep_address > GetDocument()->length()) keep_address = GetDocument()->length();
        }

        // Get current display position of point
        keep_point = ConvertToDP(addr2pos(keep_address, keep_row));

        // Recalc display based on new window size
        recalc_display();

        CPointAp newpos = addr2pos(keep_address, keep_row);  // Move caret to same byte as before resize

        // Now try to adjust scroll position to keep caret (or center line) in the window
        newpos.x = GetScroll().x;
        newpos.y -= keep_point.y;
        if (newpos.y < 0) newpos.y = 0;
        SetScroll(newpos);
    }
    else
        recalc_display();

    if (caret_displayed) DisplayCaret(); // Keep caret within display

    CScrView::OnSize(nType, cx, cy);

    // Make sure we have all the search occurrences for the new window size
    ValidateScroll(GetScroll());

    win_size_ = ConvertFromDP(CSize(cx, cy));
    // Keep track of current window size
//    win_size_.cx = cx;
//    win_size_.cy = cy;
//    {
//        CClientDC dc(this);
//        OnPrepareDC(&dc);
//        dc.DPtoLP(&win_size_);
//    }
}

void CHexEditView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
    CScrView::OnHScroll(nSBCode, nPos, pScrollBar);
    if (nSBCode != SB_THUMBTRACK)
        ((CHexEditApp *)AfxGetApp())->SaveToMacro(km_hscroll, (nSBCode << 16) | nPos);
}

void CHexEditView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
    CScrView::OnVScroll(nSBCode, nPos, pScrollBar);
    if (nSBCode != SB_THUMBTRACK)
        ((CHexEditApp *)AfxGetApp())->SaveToMacro(km_vscroll, (nSBCode << 16) | nPos);
}

BOOL CHexEditView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
    if ((nFlags & MK_CONTROL) != 0)
    {
        if (zDelta < 0)
            OnFontInc();
        else if (zDelta > 0)
            OnFontDec();
        return TRUE;
    }
    else
        return CScrView::OnMouseWheel(nFlags, zDelta, pt);
}

void CHexEditView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    if ((nFlags & 0x2100) == 0)
    {
        for (UINT ii = 0; ii < nRepCnt; ++ii)
            do_char(nChar);
    }
    else
        CScrView::OnChar(nChar, nRepCnt, nFlags);
}

void CHexEditView::do_char(UINT nChar)
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());

    FILE_ADDRESS start_addr, end_addr;
    GetSelAddr(start_addr, end_addr);
    int row = 0;
    if (start_addr == end_addr && display_.vert_display)
        row = pos2row(GetCaret());

    // Test if edit key (not special key and not BS, ^\ or nul)
    // Note: nul byte is just ignored and ^\ is only used in debug version
    if (strchr("\b\034\0", nChar) == NULL)
    {
        unsigned char cc = '\0';
        num_del_ = num_bs_ = 0;         // We're not deleting

        // Warn of a number of different problems
        if (( display_.vert_display && row > 0 ||
             !display_.vert_display && !display_.edit_char) &&
            !isxdigit(nChar))
        {
            // Non hex digit typed in hex area
#ifdef SYS_SOUNDS
             CSystemSound::Play("Invalid Character");
#else
            ::Beep(5000,200);
#endif
            aa->mac_error_ = 5;
            return;
        }
        if (( display_.vert_display && row == 0 ||
             !display_.vert_display && display_.edit_char) &&
            display_.char_set == CHARSET_EBCDIC && (nChar > 128 || a2e_tab[nChar] == 0))
        {
            ::HMessageBox("The key is not a valid EBCDIC character");
            aa->mac_error_ = 10;
            return;
        }

        if (check_ro("edit"))
            return;

        if (display_.overtype && start_addr != end_addr)
        {
			// xxx direct warning if GetDocument()->IsDevice()?
            if (::HMessageBox("This will delete the current selection\r"
                              "which is not allowed in overtype mode.\r"
                              "Do you want to turn off overtype mode?",
                              MB_OKCANCEL) == IDCANCEL)
            {
                aa->mac_error_ = 10;
                return;
            }
            else if (!do_insert())
				return;
        }
        else if (display_.overtype && start_addr == GetDocument()->length())
        {
			// xxx direct warning if GetDocument()->IsDevice()?
            if (::HMessageBox("You can't extend this file while in overtype mode.\r"
                              "Do you want to turn off overtype mode?",
                              MB_OKCANCEL) == IDCANCEL)
            {
                aa->mac_error_ = 5;
                return;
            }
            else if (!do_insert())
				return;
        }

        if ((num_entered_ % 2) == 0)
            DisplayCaret();                 // Make sure start is visible

        // If there is a selection then delete it
        if (start_addr != end_addr)
        {
            ASSERT(start_addr >= 0 && end_addr <= GetDocument()->length() && start_addr < end_addr);
            GetDocument()->Change(mod_delforw, start_addr, end_addr - start_addr,
                                  NULL, 0, this);
        }

        if ( display_.vert_display && row == 0 ||
            !display_.vert_display && display_.edit_char)
        {
            // In char area so insert/replace char at this curr locn
            if (display_.char_set == CHARSET_EBCDIC && nChar < 128)
                cc = a2e_tab[nChar];
            else if (display_.char_set == CHARSET_EBCDIC)
                cc = '\0';
            else
                cc = (unsigned char)nChar;
            GetDocument()->Change(
                display_.overtype ? mod_replace : mod_insert,
                start_addr, 1, &cc, num_entered_, this,
                start_addr != end_addr);
            // Move the caret to the next char
            ++start_addr;
            num_entered_ += 2;  // One char == 2 nybbles
        }
        else if (display_.vert_display && row == 1)
        {
            ASSERT((num_entered_ % 2) == 0);
            // Convert to hex and store in top nybble
            cc = '\0';
            if (start_addr < GetDocument()->length())
            {
                // If not at end of doc then get nybble to shift
                size_t got = GetDocument()->GetData(&cc, 1, start_addr);
                ASSERT(got == 1);
            }
            cc = (cc & 0x0F) |
                ((isdigit(nChar) ? nChar - '0' : toupper(nChar) - 'A' + 10) << 4);
            GetDocument()->Change(
                display_.overtype ? mod_replace : mod_insert,
                start_addr, 1, &cc, num_entered_, this,
                start_addr != end_addr);

            // Move to bottom nybble
            row = 2;
            ++num_entered_;
        }
        else if (display_.vert_display && row == 2)
        {
            // Bottom nybble
            cc = '\0';
            if (start_addr < GetDocument()->length())
            {
                // If not at end of doc then get nybble to shift
                size_t got = GetDocument()->GetData(&cc, 1, start_addr);
                ASSERT(got == 1);
                // Check if started entering values on 2nd (bottom) nybble
                if (num_entered_ == 0)
                {
                    // Fake entry of first (top nybble)
                    GetDocument()->Change(
                        display_.overtype ? mod_replace : mod_insert,
                        start_addr, 1, &cc, num_entered_, this);
                    ++num_entered_;
                }

                ASSERT((num_entered_ % 2) != 0);
            }
            cc = (cc & 0xF0) |
                (isdigit(nChar) ? nChar - '0' : toupper(nChar) - 'A' + 10);
            GetDocument()->Change(
                display_.overtype ? mod_replace : mod_insert,
                start_addr, 1, &cc, num_entered_, this);

            // Move to first nybble of next byte
            start_addr++;
            row = 1;
            ++num_entered_;
        }
        else if ((num_entered_ % 2) == 0)
        {
            //  First nybble entered - convert hex to low nybble
            cc = isdigit(nChar) ? nChar - '0' : toupper(nChar) - 'A' + 10;
            GetDocument()->Change(
                display_.overtype ? mod_replace : mod_insert,
                start_addr, 1, &cc, num_entered_, this,
                start_addr != end_addr);
            ++num_entered_;
        }
        else
        {
            // 2nd nybble - shift over 1st nybble and add this nybble
            cc = '\0';
            if (start_addr < GetDocument()->length())
            {
                // If not at end of doc then get nybble to shift
                size_t got = GetDocument()->GetData(&cc, 1, start_addr);
                ASSERT(got == 1);
            }
            cc = (cc << 4) + 
                (isdigit(nChar) ? nChar - '0' : toupper(nChar) - 'A' + 10);
            GetDocument()->Change(
                display_.overtype ? mod_replace : mod_insert,
                start_addr, 1, &cc, num_entered_, this);
            // Move the caret to the next byte
            ++start_addr;
            ++num_entered_;
        }
        SetCaret(addr2pos(start_addr, row));
        show_prop();                            // Current char under caret may have changed so update prop dlg
        show_calc();
        show_pos();                             // Update tool bar
        if ((num_entered_ % 2) == 0)
            DisplayCaret();                     // Make sure end is visible
    }
#if 0
    else if (nChar == '\f')                             // ^L - centre caret and redraw
    {
        CRect cli;

        // Work out window height in logical units
        GetClientRect(&cli);
        CRectAp doc_rect = ConvertFromDP(cli);

        // Move display so that caret is centred vertically
        CPointAp pp = GetCaret();
        pp.x = 0;                       // Scroll to left side (but see DisplayCaret() below)
        if ((pp.y -= (doc_rect.bottom-doc_rect.top)/2) < 0)
            pp.y = 0;
        SetScroll(pp);

        DoInvalidate();                 // Also redraw display
        DisplayCaret();                 // Make sure caret not off right side
    }
    else if (nChar == '\t' && !display_.vert_display && display_.char_area && display_.hex_area)
    {
        // Allow tab between hex and char areas if they are both visible
        num_entered_ = num_del_ = num_bs_ = 0;  // Turn off any consec edits

        // TAB key swaps between hex and char areas
        FILE_ADDRESS start_addr, end_addr;
        BOOL end_base = GetSelAddr(start_addr, end_addr);
        undo_.push_back(view_undo(undo_state));
        undo_.back().disp_state = disp_state_;
        display_.edit_char = !display_.edit_char;
        if (end_base)
            SetSel(addr2pos(end_addr), addr2pos(start_addr), true);
        else
            SetSel(addr2pos(start_addr), addr2pos(end_addr));
        DisplayCaret();
    }
    else if (nChar == '\025')                   // ^U - scroll up
    {
        CPointAp pp = GetScroll();
        pp.y -= text_height_;
        SetScroll(pp);
    }
    else if (nChar == '\004')                   // ^D - scroll down
    {
        CPointAp pp = GetScroll();
        pp.y += text_height_;
        SetScroll(pp);
    }
    else if (nChar == '\r')
    {
        num_entered_ = num_del_ = num_bs_ = 0;  // Turn off any consec edits
        FILE_ADDRESS start_addr, end_addr;
        GetSelAddr(start_addr, end_addr);

        // Check if offset has actually changed
        if (real_offset_ != (rowsize_ - start_addr%rowsize_)%rowsize_)
        {
            undo_.push_back(view_undo(undo_offset));
            undo_.back().rowsize = real_offset_;        // Save previous offset for undo
            real_offset_ = offset_ = (rowsize_ - start_addr%rowsize_)%rowsize_;
            SetSel(addr2pos(start_addr), addr2pos(end_addr));
            recalc_display();
            DoInvalidate();
        }
        else
        {
            ((CMainFrame *)AfxGetMainWnd())->
                StatusBarText("Warning: offset not changed");
            aa->mac_error_ = 1;
        }
    }
#endif
    else if (nChar == '\b')                     // Back space
    {
        // Warn if view is read only
        if (check_ro("backspace"))
            return;

        if (start_addr != end_addr)
        {
            // There is a selection - back space just deletes it
            if (display_.overtype)
            {
				// xxx direct warning if GetDocument()->IsDevice()?
                if (::HMessageBox("This will delete the current selection\r"
                                  "which is not allowed in overtype mode.\r"
                                  "Do you want to turn off overtype mode?",
                                  MB_OKCANCEL) == IDCANCEL)
                {
                    aa->mac_error_ = 5;
                    return;
                }
				else if (!do_insert())
					return;
            }

            ASSERT(start_addr >= 0 && end_addr <= GetDocument()->length() && start_addr < end_addr);
            GetDocument()->Change(mod_delforw, start_addr, end_addr - start_addr,
                                  NULL, 0, this);
        }
        else
        {
            // Back space deletes/replaces previous byte
            if ((num_entered_ % 2) != 0)        // Check if in middle of byte entry
                ++start_addr;
            if (start_addr < 1)                 // We can't back space at the start of file
            {
                aa->mac_error_ = 2;
                return;
            }
            start_addr--;                       // Work out the address of the byte to delete
            if (!display_.overtype)
            {
                // Delete previous char
                GetDocument()->Change(mod_delback, start_addr, 1,
                                  NULL, num_bs_, this);
                num_bs_ += 2;
            }
            else
            {
                // Replace previous char in OVR mode with space or nul byte
                unsigned char cc;

                if ( display_.vert_display && row == 0 || 
                    !display_.vert_display && display_.edit_char)
                {
                    // Use EBCDIC or ASCII space
                    cc = display_.char_set == CHARSET_EBCDIC ? 0x40 : 0x20;
                    GetDocument()->Change(mod_repback, start_addr, 1,
                                          &cc, num_bs_, this);
                }
                else
                {
                    ASSERT(display_.hex_area);
                    cc = '\0';
                    GetDocument()->Change(mod_repback, start_addr, 1,
                                          &cc, num_bs_, this);
                }
                ++num_bs_;
            }
        }
        num_del_ = num_entered_ = 0;                    // turn off any byte entry

        // Move the caret
        SetCaret(addr2pos(start_addr));
        show_prop();                            // New char - check props
        show_calc();
        show_pos();                             // Update tool bar
        DisplayCaret();                         // Make sure caret is visible
    }
#ifdef _DEBUG
    else if (nChar == '\034')
    {
        // Just delay for 3 seconds
        timer tt(true);
        // This loop is (hopefully) not optimised away in debug mode (optimisations off)
        while (tt.elapsed () < 2)
            ;
        ((CMainFrame *)AfxGetMainWnd())->StatusBarText("Ctrl+\\");
    }
#endif
    aa->SaveToMacro(km_char, nChar);
}

void CHexEditView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    if (nChar >= VK_PRIOR && nChar <= VK_DELETE)
    {
        // Cursor movement - reset consec. entered count
        num_entered_ = num_del_ = num_bs_ = 0;

        CPointAp start, end;
        if (GetSel(start, end))
            SetSel(end, start, true);
        else
            SetSel(start, end);     // Calls ValidateCaret - causing caret selection to be moved to valid locations
    }

    CScrView::OnKeyDown(nChar, nRepCnt, nFlags);

    if (nChar == VK_SHIFT && (nFlags & 0x4000) == 0)
    {
        // Key down and not autorepeat
        reset_tip();
        shift_moves_ = 0;
    }
    update_sel_tip();

	// Also make sure info tip is hidden
#ifdef NEW_TIPS
	tip_.Hide();
#else
	tip_.Hide(0);
#endif
}

void CHexEditView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    CScrView::OnKeyUp(nChar, nRepCnt, nFlags);

    if (nChar == VK_SHIFT)
    {
        update_sel_tip();       // Make sure window hidden when shift released

        if (theApp.highlight_ && shift_moves_)
        {
            FILE_ADDRESS start_addr, end_addr;
            GetSelAddr(start_addr, end_addr);

            add_highlight(start_addr, end_addr, TRUE);
        }
    }
}

void CHexEditView::OnKillFocus(CWnd* pNewWnd) 
{
    CScrView::OnKillFocus(pNewWnd);
    if (text_height_ == 0)
        return;

    // Save current address in case edit controls move caret
    GetSelAddr(saved_start_, saved_end_);

//    CScrView::OnKillFocus(pNewWnd);

    if (GetView() == NULL)
    {
        CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();

        mm->m_wndProp.m_pSheet->Update(NULL, -1);
    }

    // Invalidate the current selection so its drawn lighter in inactive window
    FILE_ADDRESS start_addr, end_addr;
    GetSelAddr(start_addr, end_addr);
    if (start_addr != end_addr)
        invalidate_addr_range(start_addr, end_addr);
    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing
}

void CHexEditView::OnSetFocus(CWnd* pOldWnd) 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());

    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing
    CScrView::OnSetFocus(pOldWnd);

    if (text_height_ < 1)
        return;

    show_prop();
    show_calc();
    show_pos();
    move_dlgs();

    FILE_ADDRESS start_addr, end_addr;
    GetSelAddr(start_addr, end_addr);
    if (start_addr != end_addr)
    {
        invalidate_addr_range(start_addr, end_addr);
    }

    // Save nav info in case we need to create a nav pt here
	if (theApp.navman_.LastView() != this && !theApp.navman_.InMove())
	{
        nav_start_  = start_addr;
        nav_end_    = end_addr;
		nav_scroll_ = (GetScroll().y/line_height_)*rowsize_ - offset_;
        theApp.navman_.Add("Change window", get_info(), this, nav_start_, nav_end_, nav_scroll_);
	}

    // km_new now seems to get recorded before km_focus - so check if km_new_str is there
    // (which always follows km_new) and don't store km_focus
    if (!(aa->recording_ && aa->mac_.size() > 0 && (aa->mac_.back()).ktype == km_new_str) &&
        aa->pview_ != this)
    {
        // We only want to store focus change in a macro, if the last CHexEditView that
        // got focus is different to this one.  This allows us to ignore focus changes
        // when other things (find dialog, toolbar buttons etc) temp. get focus.
        // Also this macro entry may be deleted if followed by km_new, km_open,
        // km_win_new, km_childsys (next/prev win or win close) or km_win_next
        // since these implicitly change the focus, and setting it explicitly before
        // to a certain window may ruin their effect.
        ASSERT(GetFrame() != NULL);
        CString ss;
        GetFrame()->GetWindowText(ss);
        if (ss.Right(2) == " *")
            ss = ss.Left(ss.GetLength() - 2);

        // Ignore setfocus before frame has name
        if (ss.GetLength() > 0)
            aa->SaveToMacro(km_focus, ss);
    }
    aa->pview_ = this;
}

void CHexEditView::OnDestroy() 
{
    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing
    CScrView::OnDestroy();

    // If there are no more views active ...
    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();
    if (GetView() == NULL)
    {
        // If this is the last window we need to make sure that all file
        // access buttons are disabled in the calculator (if visible)
        show_calc();
    }
}

void CHexEditView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
#if 0 // Allow BCG customisation of double-click
    // Mark position of double click (should be caret position)
    OnMark();
    CScrView::OnLButtonDblClk(nFlags, point);
#else
    if (point.x < hex_pos(0))
        theApp.OnViewDoubleClick(this, IDR_CONTEXT_ADDRESS);
    else if (display_.vert_display)
    {
        CPointAp pp = ConvertFromDP(point);          // Convert point to doc coords
        if (pp.y%line_height_ < text_height_)
            theApp.OnViewDoubleClick(this, IDR_CONTEXT_CHAR); // click on top (char) row
        else
            theApp.OnViewDoubleClick(this, IDR_CONTEXT_HEX);  // click on one of bottom 2 rows
    }
    else if (!display_.char_area || point.x < char_pos(0))
        theApp.OnViewDoubleClick(this, IDR_CONTEXT_HEX);
    else
        theApp.OnViewDoubleClick(this, IDR_CONTEXT_CHAR);
#endif
}

void CHexEditView::OnLButtonDown(UINT nFlags, CPoint point) 
{
    CPointAp pp = ConvertFromDP(point);          // Point in our coord system

    // change to allow select in address area (commented out below)
    // so that line select can be hooked up to BCG double-click in address area
    if ( /* pp.x <= (addr_width_ - 1)*text_width_ || */
        ( (display_.vert_display || display_.char_area) && pp.x >= char_pos(rowsize_)+text_width_ ||
         !(display_.vert_display || display_.char_area) && pp.x >= hex_pos(rowsize_) )  )
    {
        // Don't do anything if /* in address area on left or */ off to right
        return;
    }

    saved_state_ = disp_state_;                 // Keep the current state for saving in undo stack

    // Save some info that may be needed for macro recording
    dev_down_ = point;                          // Mouse down posn (device coords)
    doc_down_ = pp;                             // Mouse down posn (doc coords)
    shift_down_ = shift_down();

    GetSelAddr(prev_start_, prev_end_);
    prev_row_ = 0;
    if (prev_start_ == prev_end_ && display_.vert_display)
        prev_row_ = pos2row(GetCaret());

    if (!shift_down_)
    {
        num_entered_ = num_del_ = num_bs_ = 0;  // Can't be editing while mousing
        if (!display_.vert_display && display_.char_area && display_.hex_area)
        {
            // Allow user to move caret between hex and char areas

            // Which area did the user click in?
            // Note display_.edit_char must be set before ValidateCaret() is called while the
            // mouse button is held down so that the dragged selection is drawn correctly.
            display_.edit_char = pos_hex(pp.x, FALSE) >= rowsize_;

            if (saved_state_ != disp_state_)
            {
                if (display_.edit_char)
                    SetHorzBufferZone(1);
                else
                    SetHorzBufferZone(2);
            }
        }
    }

    CScrView::OnLButtonDown(nFlags, point);

    reset_tip();
    show_prop();
    show_calc();
    show_pos();
    mouse_down_ = true;                         // We saw left button down event
}

void CHexEditView::OnLButtonUp(UINT nFlags, CPoint point) 
{
    if (mouse_down_)
    {
        num_entered_ = num_del_ = num_bs_ = 0;  // Can't be editing while mousing

        CScrView::OnLButtonUp(nFlags, point);
        FILE_ADDRESS start_addr, end_addr;
        GetSelAddr(start_addr, end_addr);

        // Save in undo array if position moved
        if (prev_start_ != start_addr || prev_end_ != end_addr || saved_state_ != disp_state_)
        {
            // Save the caret position (or start of selection)
            undo_.push_back(view_undo(undo_move));
            undo_.back().address = prev_start_ | (FILE_ADDRESS(prev_row_)<<62);
            if (prev_start_ != prev_end_)
            {
                // Save the previous selection
                undo_.push_back(view_undo(undo_sel));
                undo_.back().address = prev_end_;
            }
            if (saved_state_ != disp_state_)
            {
                // Save the fact that we swapped between areas
                undo_.push_back(view_undo(undo_state, TRUE));
                undo_.back().disp_state = saved_state_;
            }

            // Save info to keystroke macro (if recording)
            CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
            if (aa->recording_)
            {
                mouse_sel ms;                   // Info for km_mouse/km_shift_mouse
                ms.dev_down = dev_down_;        // Point in window where selection started
                CPointAp doc_up = ConvertFromDP(point); // Point where selection ended (doc coords)

//              ms.doc_dist = doc_up - doc_down_;
                // To avoid problems where a different number of rows are selected on play
                // for slightly different scroll positions we round up to mult. of text_height_.
                ms.doc_dist.cy = (doc_up.y/text_height_ - doc_down_.y/text_height_) * text_height_;
//                ms.doc_dist.cx = (doc_up.x/text_width_ - doc_down_.x/text_width_) * text_width_;
                if (shift_down_)
                    aa->SaveToMacro(km_shift_mouse, &ms);
                else
                    aa->SaveToMacro(km_mouse, &ms);
            }

            if (aa->highlight_)
                add_highlight(start_addr, end_addr, TRUE);

			nav_save(start_addr, end_addr, "Mouse Click");
        }

        show_prop();
        show_calc();
        show_pos();
        mouse_down_ = false;
        update_sel_tip();           // Make sure window hidden when mouse button up
    }
}

void CHexEditView::OnMouseMove(UINT nFlags, CPoint point) 
{
    CScrView::OnMouseMove(nFlags, point);

    if (mouse_down_)
    {
#ifdef NEW_TIPS
		tip_.Hide();                    // Make sure there is no mouse tip while dragging
#else
		tip_.Hide(0);
#endif
        move_dlgs();
        update_sel_tip(200);                // Update selection tip
    }
    else if (!tip_.IsWindowVisible())
    {
        // Set a timer so that we can check if we want to display a tip window
        track_mouse(TME_HOVER);
    }
	else if (address_at(point) != last_tip_addr_ && tip_.IsWindowVisible())
    {
        // Hide the tip window since the mouse has moved away
#ifdef NEW_TIPS
		if (!tip_.FadingOut())   // Don't keep it alive if we have already told it to fade away
			tip_.Hide();
#else
		tip_.Hide(0);
#endif
    }
}

/*
void CHexEditView::OnTimer(UINT nIDEvent)
{
    if (nIDEvent == timer_id_)
    {
        VERIFY(KillTimer(timer_id_));
        timer_id_ = 0;

        CPoint pt;
        GetCursorPos(&pt);
        ScreenToClient(&pt);

        if (pt != last_mouse_)
            return;

        // Time to show a tip window if we are in the right place
		FILE_ADDRESS addr = address_at(pt);
		int bm;
        if (addr != -1 && (bm = bookmark_at(addr)) != -1)
        {
			last_tip_addr_ = addr;
            CBookmarkList *pbl = theApp.GetBookmarkList();
            tip_.SetWindowText(pbl->name_[GetDocument()->bm_index_[bm]]);
			CPoint point = last_mouse_ + CSize(text_width_w_, text_height_);
		    ClientToScreen(&point);
            tip_.Move(point, false);
#ifdef NEW_TIPS
            tip_.Show();
#else
            tip_.Show(0, 0);
#endif
        }
    }
    else
        CScrView::OnTimer(nIDEvent);
}
*/

LRESULT CHexEditView::OnMouseHover(WPARAM, LPARAM lp)
{
    // Time to show a tip window if we are in the right place
	CPoint pt(LOWORD(lp), HIWORD(lp));
	FILE_ADDRESS addr = address_at(pt);
	if (addr != -1 && addr < GetDocument()->length() && update_tip(addr))
	{
		last_tip_addr_ = addr;
		pt += CSize(text_width_w_, text_height_);
		ClientToScreen(&pt);
        tip_.Move(pt, false);
#ifdef NEW_TIPS
        tip_.Show();
#else
        tip_.Show(0, 0);
#endif
		track_mouse(TME_LEAVE);
    }
    return 0;
}

// Returns true if tip text was updated or false if there is nothing to show.
// The parameter (addr) if the address about which we show information
bool CHexEditView::update_tip(FILE_ADDRESS addr)
{
	bool retval = false;
	ExprStringType ss;
	tip_addr_ = addr;

	ASSERT(theApp.tip_name_.size() > 0); // we should at leats have bookmarks
	ASSERT(theApp.tip_on_    .size() == theApp.tip_name_.size());
	ASSERT(theApp.tip_expr_  .size() == theApp.tip_name_.size());
	ASSERT(theApp.tip_format_.size() == theApp.tip_name_.size());

	for (size_t ii = 0; ii < theApp.tip_name_.size(); ++ii)
		if (theApp.tip_on_[ii])
		{
			if (retval)
				ss += "\r\n";   // start this tip on a new line

			if (ii == 0)
			{
				// Special (hard-coded) bookmark tip
				ASSERT(theApp.tip_name_[ii] == "Bookmarks");

				int bm;              // Index into doc's bookmark list of bookmark under cursor
				if ((bm = bookmark_at(addr)) != -1)
				{
					ss += "Bookmark: " + theApp.GetBookmarkList()->name_[GetDocument()->bm_index_[bm]];
					retval = true;
				}
			}
			// Add any more "hard-coded" tip types here ...

			else
			{
				ASSERT(ii >= FIRST_USER_TIP);
				ss += theApp.tip_name_[ii] + ": ";
				int dummy;
				expr_eval::value_t val = expr_.evaluate(theApp.tip_expr_[ii], 0, dummy);
				ss += GetDocument()->GetDataString(val, theApp.tip_format_[ii], expr_.GetSize(), expr_.GetUnsigned());
				retval = true;
			}
		}

#ifdef UNICODE_TYPE_STRING
	// This is the way to put Unicode text into control with ANSI window procedure
	// xxx this doesn't work for a CTipWnd for some reason - we need to fix this so we can display Unicode char/string
	//::CallWindowProcW(*tip_.GetSuperWndProcAddr(), tip_.m_hWnd, WM_SETTEXT, 0, (LPARAM)(LPCWSTR)ss);
	tip_.SetWindowText(CString(ss));
#else
	tip_.SetWindowText(ss);
#endif
	return retval;
}

LRESULT CHexEditView::OnMouseLeave(WPARAM, LPARAM lp)
{
#ifdef NEW_TIPS
	tip_.Hide();
#else
	tip_.Hide(0);
#endif
    return 0;
}

void CHexEditView::OnSysColorChange()
{
    CScrView::OnSysColorChange();
    set_colours();
    Invalidate();
}


void CHexEditView::track_mouse(unsigned long flag)
{
    TRACKMOUSEEVENT tme;
    tme.cbSize = sizeof(tme);
    tme.dwFlags = flag;
    tme.dwHoverTime = 0;
    tme.hwndTrack = m_hWnd;

    VERIFY(::_TrackMouseEvent(&tme));
}

FILE_ADDRESS CHexEditView::address_at(CPoint pt)
{
    CPointAp doc_pt = ConvertFromDP(pt);     // Mouse position (doc coords)

    if (doc_pt.x < hex_pos(0) || doc_pt.x > char_pos(rowsize_)+text_width_)
        return -1;                           // Left of hex area (ie in address area)
    else if ( (display_.vert_display || display_.char_area) && doc_pt.x >= char_pos(rowsize_)+text_width_ ||
             !(display_.vert_display || display_.char_area) && doc_pt.x >= hex_pos(rowsize_) )
        return -1;                           // Right of areas (ie in blank area)
    else if (display_.vert_display)
        return (doc_pt.y/line_height_)*rowsize_ - offset_ + pos_char(doc_pt.x, TRUE);
    else if (!display_.char_area || doc_pt.x < char_pos(0))
        return (doc_pt.y/text_height_)*rowsize_ - offset_ + pos_hex(doc_pt.x, TRUE);
    else
        return (doc_pt.y/text_height_)*rowsize_ - offset_ + pos_char(doc_pt.x, TRUE);
}

// Returns the bookmark at file address or -1 if none
int CHexEditView::bookmark_at(FILE_ADDRESS addr)
{
    std::vector<FILE_ADDRESS>::const_iterator pp =
        std::find(GetDocument()->bm_posn_.begin(), GetDocument()->bm_posn_.end(), addr);
    if (pp != GetDocument()->bm_posn_.end())
        return pp - GetDocument()->bm_posn_.begin();
    else
        return -1;
}

void CHexEditView::OnHighlight() 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());

    FILE_ADDRESS start_addr, end_addr;
    GetSelAddr(start_addr, end_addr);

    if (aa->highlight_)
        aa->highlight_ = FALSE;
    else if (start_addr != end_addr)
        add_highlight(start_addr, end_addr, FALSE);
    else
        aa->highlight_ = TRUE;
    aa->SaveToMacro(km_highlight, (unsigned __int64)1);
}

void CHexEditView::OnUpdateHighlight(CCmdUI* pCmdUI) 
{
    pCmdUI->SetCheck(dynamic_cast<CHexEditApp *>(AfxGetApp())->highlight_);
}

void CHexEditView::OnHighlightClear() 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
    aa->highlight_ = FALSE;
    if (!hl_set_.empty())
    {
        undo_.push_back(view_undo(undo_highlight));
        undo_.back().phl = new range_set<FILE_ADDRESS>(hl_set_);
        hl_set_.clear();
        DoInvalidate();
    }
    aa->SaveToMacro(km_highlight);
}

void CHexEditView::OnHighlightPrev() 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
    FILE_ADDRESS start_addr, end_addr;

    GetSelAddr(start_addr, end_addr);
    if (hl_set_.range_.empty())
    {
        ::HMessageBox("Nothing highlighted");
        aa->mac_error_ = 10;
        return;
    }
    else if (hl_set_.range_.front().sfirst >= start_addr)
    {
        ::HMessageBox("No highlight before start of file");
        aa->mac_error_ = 10;
        return;
    }
    else
    {
        range_set<FILE_ADDRESS>::range_t::const_iterator pp =
            lower_bound(hl_set_.range_.begin(),
                        hl_set_.range_.end(),
                        range_set<FILE_ADDRESS>::segment(start_addr, end_addr),
                        segment_compare());
        ASSERT(pp != hl_set_.range_.begin());
        pp--;           // Get the previous elt
        MoveWithDesc("Previous Highlight", pp->sfirst, pp->slast);
        aa->SaveToMacro(km_highlight, (unsigned __int64)2);
    }
}

void CHexEditView::OnUpdateHighlightPrev(CCmdUI* pCmdUI) 
{
    FILE_ADDRESS start_addr, end_addr;
    GetSelAddr(start_addr, end_addr);
    pCmdUI->Enable(!hl_set_.range_.empty() && hl_set_.range_.front().sfirst < start_addr);
}

void CHexEditView::OnHighlightNext() 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
    FILE_ADDRESS start_addr, end_addr;

    GetSelAddr(start_addr, end_addr);
    if (hl_set_.range_.empty())
    {
        ::HMessageBox("Nothing highlighted");
        aa->mac_error_ = 10;
        return;
    }
    else if (hl_set_.range_.back().sfirst <= start_addr)
    {
        ::HMessageBox("No highlight before end of file");
        aa->mac_error_ = 10;
        return;
    }
    else
    {
        range_set<FILE_ADDRESS>::range_t::const_iterator pp =
            lower_bound(hl_set_.range_.begin(),
                        hl_set_.range_.end(),
                        range_set<FILE_ADDRESS>::segment(start_addr+1, end_addr+1),
                        segment_compare());
        ASSERT(pp != hl_set_.range_.end());
        MoveWithDesc("Next Highlight", pp->sfirst, pp->slast);
        aa->SaveToMacro(km_highlight, (unsigned __int64)3);
    }
}

void CHexEditView::OnUpdateHighlightNext(CCmdUI* pCmdUI) 
{
    FILE_ADDRESS start_addr, end_addr;
    GetSelAddr(start_addr, end_addr);
    pCmdUI->Enable(!hl_set_.range_.empty() && hl_set_.range_.back().sfirst > start_addr);
}

void CHexEditView::add_highlight(FILE_ADDRESS start, FILE_ADDRESS end, BOOL ptoo /*=FALSE*/)
{
    if (start < end)
    {
        undo_.push_back(view_undo(undo_highlight, ptoo));
        undo_.back().phl = new range_set<FILE_ADDRESS>(hl_set_);

        range_set<FILE_ADDRESS> tt = hl_set_;
        tt.insert_range(start, end);
        // If selected area is already part of highlighted area
        if (tt == hl_set_)
            hl_set_.erase_range(start, end);    // Remove it from highlight
        else
            hl_set_.swap(tt);                   // else add it to highlight
        invalidate_addr_range(start, end);
    }

#ifdef _DEBUG
    for (range_set<FILE_ADDRESS>::range_t::iterator pr = hl_set_.range_.begin(); pr != hl_set_.range_.end(); ++pr)
    {
        FILE_ADDRESS fa = pr->sfirst;
        fa = pr->slast;
    }
#endif
}

void CHexEditView::OnHighlightHide() 
{
    begin_change();
    display_.hide_highlight = !display_.hide_highlight;
    make_change();
    end_change();
    theApp.SaveToMacro(km_highlight, (unsigned __int64)4);
}

void CHexEditView::OnUpdateHighlightHide(CCmdUI* pCmdUI) 
{
    pCmdUI->SetCheck(display_.hide_highlight);
}

// We will try returning the bookmark above or -1 if there is none
int CHexEditView::ClosestBookmark(FILE_ADDRESS &diff)
{
    std::vector<FILE_ADDRESS>::const_iterator prev_bm = GetDocument()->bm_posn_.end();
    diff = GetDocument()->length()+1;  // Bigger than any possible difference between current & bookmark
    FILE_ADDRESS start_addr, end_addr;

    GetSelAddr(start_addr, end_addr);

    // Find the bookmark that is closest but after the current address
    for (std::vector<FILE_ADDRESS>::const_iterator pbm = GetDocument()->bm_posn_.begin();
         pbm != GetDocument()->bm_posn_.end(); ++pbm)
    {
        if (*pbm <= start_addr && start_addr - *pbm < diff)
        {
            diff = start_addr - *pbm;
            prev_bm = pbm;
        }
    }
    if (prev_bm == GetDocument()->bm_posn_.end())
        return -1;
    else
        return GetDocument()->bm_index_[prev_bm - GetDocument()->bm_posn_.begin()];
}

void CHexEditView::OnBookmarksClear() 
{
    GetDocument()->ClearBookmarks();
    theApp.SaveToMacro(km_bookmarks);
}

void CHexEditView::OnUpdateBookmarksClear(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(!GetDocument()->bm_posn_.empty());
}

void CHexEditView::OnBookmarksPrev() 
{
    std::vector<FILE_ADDRESS>::const_iterator prev_bm = GetDocument()->bm_posn_.end();
    FILE_ADDRESS diff = GetDocument()->length()+1;  // Bigger than any possible difference between current & bookmark
    FILE_ADDRESS start_addr, end_addr;

    GetSelAddr(start_addr, end_addr);

    // Find the bookmark that is closest but after the current address
    for (std::vector<FILE_ADDRESS>::const_iterator pbm = GetDocument()->bm_posn_.begin();
         pbm != GetDocument()->bm_posn_.end(); ++pbm)
    {
        if (*pbm < start_addr && start_addr - *pbm < diff)
        {
            diff = start_addr - *pbm;
            prev_bm = pbm;
        }
    }
    if (prev_bm == GetDocument()->bm_posn_.end())
    {
        // No bookmarks found (presumably in macro playback)
        ::HMessageBox("No bookmarks found towards start of file");
        theApp.mac_error_ = 10;
        return;
    }

    MoveWithDesc("Previous Bookmark", *prev_bm, *prev_bm);
    theApp.SaveToMacro(km_bookmarks, (unsigned __int64)2);
}

void CHexEditView::OnUpdateBookmarksPrev(CCmdUI* pCmdUI) 
{
    std::vector<FILE_ADDRESS>::const_iterator prev_bm = GetDocument()->bm_posn_.end();
    FILE_ADDRESS diff = GetDocument()->length()+1;  // Bigger than any possible difference between current & bookmark
    FILE_ADDRESS start_addr, end_addr;

    GetSelAddr(start_addr, end_addr);

    // Find the bookmark that is closest but after the current address
    for (std::vector<FILE_ADDRESS>::const_iterator pbm = GetDocument()->bm_posn_.begin();
         pbm != GetDocument()->bm_posn_.end(); ++pbm)
    {
        if (*pbm < start_addr && start_addr - *pbm < diff)
        {
            diff = start_addr - *pbm;
            prev_bm = pbm;
        }
    }
    pCmdUI->Enable(prev_bm != GetDocument()->bm_posn_.end());
}

void CHexEditView::OnBookmarksNext() 
{
    std::vector<FILE_ADDRESS>::const_iterator next_bm = GetDocument()->bm_posn_.end();
    FILE_ADDRESS diff = GetDocument()->length()+1;  // Bigger than any possible difference between current & bookmark
    FILE_ADDRESS start_addr, end_addr;

    GetSelAddr(start_addr, end_addr);

    // Find the bookmark that is closest but after the current address
    for (std::vector<FILE_ADDRESS>::const_iterator pbm = GetDocument()->bm_posn_.begin();
         pbm != GetDocument()->bm_posn_.end(); ++pbm)
    {
        if (*pbm > start_addr && *pbm - start_addr < diff)
        {
            diff = *pbm - start_addr;
            next_bm = pbm;
        }
    }
    if (next_bm == GetDocument()->bm_posn_.end())
    {
        // No bookmarks found (presumably in macro playback)
        ::HMessageBox("No bookmarks before end of file");
        theApp.mac_error_ = 10;
        return;
    }

    MoveWithDesc("Next Bookmark", *next_bm, *next_bm);
    theApp.SaveToMacro(km_bookmarks, (unsigned __int64)3);
}

void CHexEditView::OnUpdateBookmarksNext(CCmdUI* pCmdUI) 
{
    std::vector<FILE_ADDRESS>::const_iterator next_bm = GetDocument()->bm_posn_.end();
    FILE_ADDRESS diff = GetDocument()->length()+1;  // Bigger than any possible difference between current & bookmark
    FILE_ADDRESS start_addr, end_addr;

    GetSelAddr(start_addr, end_addr);

    // Find the bookmark that is closest but after the current address
    for (std::vector<FILE_ADDRESS>::const_iterator pbm = GetDocument()->bm_posn_.begin();
         pbm != GetDocument()->bm_posn_.end(); ++pbm)
    {
        if (*pbm > end_addr && *pbm - end_addr < diff)
        {
            diff = *pbm - end_addr;
            next_bm = pbm;
        }
    }
    pCmdUI->Enable(next_bm != GetDocument()->bm_posn_.end());
}

void CHexEditView::OnBookmarksHide() 
{
    begin_change();
    display_.hide_bookmarks = !display_.hide_bookmarks;
    make_change();
    end_change();
    theApp.SaveToMacro(km_bookmarks, (unsigned __int64)4);
}

void CHexEditView::OnUpdateBookmarksHide(CCmdUI* pCmdUI) 
{
    pCmdUI->SetCheck(display_.hide_bookmarks);
}

void CHexEditView::OnBookmarkToggle() 
{
    if (GetDocument()->pfile1_ == NULL)
	{
        ::HMessageBox("Bookmarks can only be added to disk files.\r\n"
			          "Please save the file to disk and try again.");
        ((CHexEditApp *)AfxGetApp())->mac_error_ = 5;
		return;
	}

    int index;
    CBookmarkList *pbl = theApp.GetBookmarkList();

    // Test if there is already a bookmark at this location
    if ((index = GetDocument()->GetBookmarkAt(GetPos())) == -1)
    {
        // Add an unnamed bookmark here
        char *dv_name = "unnamed";
        int dummy;
        long last = pbl->GetSetLast(dv_name, dummy);

        CString bm_name;
        bm_name.Format("%s%05ld", dv_name, long(last));
        pbl->AddBookmark(bm_name, GetDocument()->pfile1_->GetFilePath(),
                         GetPos(), NULL, GetDocument());

    }
    else
        pbl->RemoveBookmark(index);    // remove the bookmark

    theApp.SaveToMacro(km_bookmarks, (unsigned __int64)7);
}


void CHexEditView::reset_tip()
{
    FILE_ADDRESS start_addr, end_addr;
    GetSelAddr(start_addr, end_addr);

    CPointAp pt(addr2pos(start_addr));
    CPoint point = ConvertToDP(pt);
    point.x -= 24;
	ClientToScreen(&point);

	sel_tip_.Move(point);
#ifdef NEW_TIPS
	sel_tip_.Hide();
#else
	sel_tip_.Hide(0);
#endif
}

void CHexEditView::update_sel_tip(int delay /*=0*/)
{
    FILE_ADDRESS start_addr, end_addr;
    GetSelAddr(start_addr, end_addr);

    if ((mouse_down_ || shift_down()) && start_addr != end_addr)
    {
        CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
        CString ss;

        __int64 len = __int64(end_addr - start_addr);

        if (display_.dec_addr)
        {
            char buf[22];                    // temp buf where we sprintf
            sprintf(buf, "%I64d", __int64(len));
            ss = buf;
            AddCommas(ss);
            sel_tip_.SetTextCol(dec_addr_col_);

            // If there is a fair amount of contrast with standard tip background
            // colour then use it else use something else.
            if (abs(BRIGHTNESS(dec_addr_col_) - BRIGHTNESS(::GetSysColor(COLOR_INFOBK))) > 30)
                sel_tip_.SetBgCol(::GetSysColor(COLOR_INFOBK));
            else if (BRIGHTNESS(dec_addr_col_) > 50)
                sel_tip_.SetBgCol(RGB(64,64,0));       // Light text so use dark "yellow" bg
            else
                sel_tip_.SetBgCol(RGB(255,255,224));   // Dark text so use light yellow bg
        }
        else
        {
            char buf[22];                    // temp buf where we sprintf
            if (len < 10)
                sprintf(buf, "%I64x", __int64(len));
            else if (aa->hex_ucase_)
                sprintf(buf, "%I64Xh", __int64(len));
            else
                sprintf(buf, "%I64xh", __int64(len));
            ss = buf;
            AddSpaces(ss);
            sel_tip_.SetTextCol(hex_addr_col_);

            // If there is a fair amount of contrast with standard tip background
            // colour then use it else use something else.
            if (abs(BRIGHTNESS(hex_addr_col_) - BRIGHTNESS(::GetSysColor(COLOR_INFOBK))) > 30)
                sel_tip_.SetBgCol(::GetSysColor(COLOR_INFOBK));
            else if (BRIGHTNESS(hex_addr_col_) > 50)
                sel_tip_.SetBgCol(RGB(64,64,0));       // Light text so use dark "yellow" bg
            else
                sel_tip_.SetBgCol(RGB(255,255,224));   // Dark text so use light yellow bg
        }

#if 0  // We used to indicate divisibility using colour
        if (len%8 == 0)
            sel_tip_.SetTextCol(RGB(0, 0, 128));
        else if (len%4 == 0)
            sel_tip_.SetTextCol(RGB(128, 0, 128));
        else if (len%2 == 0)
            sel_tip_.SetTextCol(RGB(128, 128, 0));
        else
            sel_tip_.SetTextCol(RGB(128, 128, 128));
#endif
        if (len > 10)  // User should know divisibility by 2,4 for numbers this small so don't show
        {
            int pow2;

            for (pow2 = 0; pow2 < 12; ++pow2)
                if (((len>>pow2) & 0x1) != 0)
                    break;

            CString tmp;
            tmp.Format(" [%d]", 1<<pow2);

            ss += tmp;
        }

        // Don't update text if the string hasn't changed to avoid flicker
        CString strPrevious;
        sel_tip_.GetWindowText(strPrevious);
        if (strPrevious.Compare(ss) != 0)
            sel_tip_.SetWindowText(ss);

        // Check if tip window is too far from selection
        FILE_ADDRESS start_line = (start_addr + offset_)/rowsize_;
        FILE_ADDRESS end_line = (end_addr + offset_)/rowsize_ + 1;

        CRect rct;

        // Get bounding rectangle of selection (we only really need top and bottom)
        CRectAp bnd_rct;
        bnd_rct = CRectAp(0, start_line * line_height_,
                          char_pos(rowsize_), end_line * line_height_);

        sel_tip_.GetWindowRect(&rct);
        ScreenToClient(&rct);
        CRectAp tip_rct = ConvertFromDP(rct);

        GetClientRect(rct);
        CRectAp cli_rct = ConvertFromDP(rct);

        bnd_rct.top -= tip_rct.Height()/2;
        bnd_rct.bottom += tip_rct.Height()/2;

        // Restrict bounds rect to the visible selection
        if (bnd_rct.top < cli_rct.top - 10)
        {
            bnd_rct.top = cli_rct.top - 10;
            if (bnd_rct.bottom < bnd_rct.top + tip_rct.Height())
                bnd_rct.bottom = bnd_rct.top + tip_rct.Height();
        }
        if (bnd_rct.bottom > cli_rct.bottom + 10)
        {
            bnd_rct.bottom = cli_rct.bottom + 10;
            if (bnd_rct.top > bnd_rct.bottom - tip_rct.Height())
                bnd_rct.top = bnd_rct.bottom - tip_rct.Height();
        }
        if (bnd_rct.left < cli_rct.left)
            bnd_rct.left = cli_rct.left;
        if (bnd_rct.right > cli_rct.right)
            bnd_rct.right = cli_rct.right;

        // Move the rectangle for tip within bounds
        if (tip_rct.top < bnd_rct.top)
        {
            tip_rct.bottom += bnd_rct.top - tip_rct.top;
            tip_rct.top    += bnd_rct.top - tip_rct.top;
        }
        else if (tip_rct.bottom > bnd_rct.bottom)
        {
            tip_rct.top    += bnd_rct.bottom - tip_rct.bottom;
            tip_rct.bottom += bnd_rct.bottom - tip_rct.bottom;
        }
        if (tip_rct.left < bnd_rct.left)
        {
            tip_rct.right += bnd_rct.left - tip_rct.left;
            tip_rct.left  += bnd_rct.left - tip_rct.left;
        }
        else if (tip_rct.right > bnd_rct.right)
        {
            tip_rct.left  += bnd_rct.right - tip_rct.right;
            tip_rct.right += bnd_rct.right - tip_rct.right;
        }

        // Calculate centre of rect and move the tip window
        CPointAp pt((tip_rct.left + tip_rct.right)/2, (tip_rct.top + tip_rct.bottom)/2);
		CPoint point = ConvertToDP(pt);
		ClientToScreen(&point);
        sel_tip_.Move(point);

#ifdef NEW_TIPS
        sel_tip_.Show(100, delay);
#else
        sel_tip_.Show(0,0);
#endif
        SetFocus();
    }
    else
    {
#ifdef NEW_TIPS
        sel_tip_.Hide();
#else
        sel_tip_.Hide(0);
#endif
    }
}

// This is here to implement the macro command km_mouse
void CHexEditView::do_mouse(CPoint dev_down, CSizeAp doc_dist)
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());

    CPointAp doc_down = ConvertFromDP(dev_down);  // Sel start point converted from device to doc coords
    CPointAp doc_up(doc_down);                    // Point where selection ends = start +
    doc_up += doc_dist;                         // ... distance of selection

    // Convert selection (doc coords) to addresses and make sure in valid range
    FILE_ADDRESS start_addr, end_addr;
    FILE_ADDRESS length = GetDocument()->length();
    start_addr = pos2addr(doc_down);
    end_addr = pos2addr(doc_up);

    if (start_addr < 0) start_addr = 0; else if (start_addr > length) start_addr = length;
    if (end_addr < 0) end_addr = 0; else if (end_addr > length) end_addr = length;

    // Save undo information
    FILE_ADDRESS prev_start, prev_end;
    GetSelAddr(prev_start, prev_end);
    int prev_row = 0;
    if (prev_start == prev_end && display_.vert_display)
        prev_row = pos2row(GetCaret());

    if (start_addr == prev_start && end_addr == prev_end)  // exactly the same selection
        return;

    BOOL saved_area = display_.edit_char;
    if (!display_.vert_display && display_.char_area && display_.hex_area) // If both areas displayed ...
        display_.edit_char = pos_hex(doc_down.x, FALSE) >= rowsize_;       // sel may swap between areas
    undo_.push_back(view_undo(undo_move));
    undo_.back().address = prev_start | (FILE_ADDRESS(prev_row)<<62);
    if (prev_start != prev_end)
    {
        // Save the previous selection
        undo_.push_back(view_undo(undo_sel));
        undo_.back().address = prev_end;
    }
    if (display_.edit_char != saved_area)
    {
        // Allow more buffer if editing hex
        if (display_.edit_char)
            SetHorzBufferZone(1);
        else
            SetHorzBufferZone(2);
        // Save the fact that we swapped between areas
        undo_.push_back(view_undo(undo_state, TRUE));
        undo_.back().disp_state = disp_state_;
    }

    SetSel(addr2pos(start_addr), addr2pos(end_addr), true);

    if (aa->highlight_)
        add_highlight(start_addr, end_addr, TRUE);

	nav_save(start_addr, end_addr, "Mouse Click (Play)");
}

// This is here to implement the macro command km_shift_mouse
void CHexEditView::do_shift_mouse(CPoint dev_down, CSizeAp doc_dist) 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());

    CPointAp doc_down = ConvertFromDP(dev_down); // Left down point converted from device to doc coords
    CPointAp doc_up(doc_down);                   // Point of left button up (doc coords) = ...
    doc_up += doc_dist;                          // ... doc_down + length of selection

    // Convert selection (doc coords) to addresses and make sure in valid range
    FILE_ADDRESS start_addr, end_addr;                  // New selection range
    FILE_ADDRESS prev_start, prev_end;                  // Previous selection range
    FILE_ADDRESS length = GetDocument()->length();
//    GetSelAddr(start_addr, prev_end);
    if (GetSelAddr(prev_start, prev_end))
        start_addr = prev_end;                  // Base of previous selection was end
    else
        start_addr = prev_start;
    end_addr = pos2addr(doc_up);

    if (start_addr < 0) start_addr = 0; else if (start_addr > length) start_addr = length;
    if (end_addr < 0) end_addr = 0; else if (end_addr > length) end_addr = length;

    if (end_addr != prev_end)
    {
        // Save the previous selection for undo
        undo_.push_back(view_undo(undo_move));
        undo_.back().address = start_addr;
        undo_.push_back(view_undo(undo_sel));
        undo_.back().address = prev_end;

        // Extend the selection
        SetSel(addr2pos(start_addr), addr2pos(end_addr), true);

        if (aa->highlight_)
            add_highlight(start_addr, end_addr, TRUE);
    }
	nav_save(start_addr, end_addr, "Mouse Click (Play)");
}

void CHexEditView::OnRButtonDown(UINT nFlags, CPoint point) 
{
    // Move the caret to where the user clicked so that context menu items
    // that depend on the current address use the address that was clicked on
    CPointAp pp = ConvertFromDP(point);          // Point in our coord system

    if (( (display_.vert_display || display_.char_area) && pp.x >= char_pos(rowsize_)+text_width_ ||
         !(display_.vert_display || display_.char_area) && pp.x >= hex_pos(rowsize_) )  )
    {
        // Don't do anything if off to right
        return;
    }

    // Save current state, selection etc for undo
    GetSelAddr(prev_start_, prev_end_);
    saved_state_ = disp_state_;                 // Keep the current state for saving in undo stack
    prev_row_ = 0;
    if (prev_start_ == prev_end_ && display_.vert_display)
        prev_row_ = pos2row(GetCaret());

    num_entered_ = num_del_ = num_bs_ = 0;  // Can't be editing while mousing
    if (!display_.vert_display && display_.char_area && display_.hex_area)
    {
        // Allow user to move caret between hex and char areas
        display_.edit_char = pos_hex(pp.x, FALSE) >= rowsize_;

        if (saved_state_ != disp_state_)
        {
            // Sett approp. buffering to allow user to see the whole byte that they are editing
            if (display_.edit_char)
                SetHorzBufferZone(1);
            else
                SetHorzBufferZone(2);
        }
    }

    // Note: this relies on display_.edit_char having been set correctly above
    FILE_ADDRESS addr = pos2addr(pp);
    FILE_ADDRESS start_addr, end_addr;
    BOOL end_base = FALSE;

    // If we have not clicked inside the selection then ...
    if (addr < prev_start_ || addr >= prev_end_)
    {
        // Move the caret to where the user clicked
        ValidateCaret(pp, FALSE);
        SetCaret(pp);
        BOOL end_base = GetSelAddr(start_addr, end_addr);
    }
    else
    {
        // Don't move if clicked in selection
        start_addr = prev_start_;
        end_addr = prev_end_;
    }

    // Save in undo array if position moved
    if (prev_start_ != start_addr || prev_end_ != end_addr || saved_state_ != disp_state_)
    {
        // Save the caret position (or start of selection)
        undo_.push_back(view_undo(undo_move));
        undo_.back().address = prev_start_ | (FILE_ADDRESS(prev_row_)<<62);
        if (prev_start_ != prev_end_)
        {
            // Save the previous selection
            undo_.push_back(view_undo(undo_sel));
            undo_.back().address = prev_end_;
        }
        if (saved_state_ != disp_state_)
        {
            // Save the fact that we swapped between areas
            undo_.push_back(view_undo(undo_state, TRUE));
            undo_.back().disp_state = saved_state_;
        }

        if (end_base)
            SetSel(addr2pos(end_addr), addr2pos(start_addr), true);
        else
            SetSel(addr2pos(start_addr), addr2pos(end_addr));
        DisplayCaret();

		nav_save(start_addr, end_addr, "Right Mouse Click");
    }

    // Update properties etc for new position
    show_prop();
    show_calc();
    show_pos();


#if 0
    else if (saved_state_ != disp_state_)
    {
        // Save the fact that we swapped between areas
        undo_.push_back(view_undo(undo_state, FALSE));
        undo_.back().disp_state = saved_state_;
    }
--------
    FILE_ADDRESS addr = pos2addr(pp);
    FILE_ADDRESS start_addr, end_addr;
    GetSelAddr(start_addr, end_addr);

    // Make sure we have NOT clicked inside the selection
    if (addr < start_addr || addr > end_addr)
    {
        // Move the caret to where the user clicked
        ValidateCaret(pp, FALSE);
        SetCaret(pp);

        // Update properties etc for new position
        show_prop();
        show_calc();
        show_pos();
    }
#endif

    CScrView::OnRButtonDown(nFlags, point);
}

void CHexEditView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    view_context(point);
}

BOOL CHexEditView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
    // Only respond if in client area
    if (nHitTest == HTCLIENT)
    {
        // Work out location of cursor in window and make sure it's
        // not over address area (on left) or past last chars (on right)
        CPoint point;
        ::GetCursorPos(&point);
        ScreenToClient(&point);
        CPointAp pt = ConvertFromDP(point);
        if (pt.x > (addr_width_ - 1)*text_width_ &&
            (display_.vert_display && pt.x < char_pos(rowsize_)+text_width_w_ ||
             !display_.vert_display && display_.char_area && pt.x < char_pos(rowsize_)+text_width_w_ ||
             !display_.vert_display && !display_.char_area && pt.x < hex_pos(rowsize_) )  )
        {
            // Over hex or char area
            if (theApp.highlight_)
                ::SetCursor(theApp.LoadCursor(IDC_HIGHLIGHT));       // Highlighter cursor
            else
                ::SetCursor(theApp.LoadStandardCursor(IDC_IBEAM));   // Text insertion cursor
            return TRUE;
        }
    }
        
    return CScrView::OnSetCursor(pWnd, nHitTest, message);
}

// Display views context menu
void CHexEditView::view_context(CPoint point) 
{
#if 0 // Changes for BCG context menus
    // Get the top level menu that contains the submenus used as popup menus
    CMenu top;
    BOOL ok = top.LoadMenu(IDR_CONTEXT);
    ASSERT(ok);
    if (!ok) return;

    CHECK_SECURITY(51);

    CMenu *ppop;
    if (point.x < hex_pos(0))
    {
        // Context menu for address area
        ppop = top.GetSubMenu(1);
    }
    else if (display_.char_area && point.x >= char_pos(0))
    {
        // Context menu for character area
        ppop = top.GetSubMenu(2);
    }
    else
    {
        // Context menu for hex area
        ppop = top.GetSubMenu(3);
    }
    ASSERT(ppop != NULL);
    if (ppop != NULL)
    {
        // Convert window coords to the required screen coords
        ClientToScreen(&point);
        VERIFY(ppop->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                                    point.x, point.y, GetFrame()));
    }

    top.DestroyMenu();
#else
    CBCGContextMenuManager *pCMM = theApp.GetContextMenuManager();

    CPoint cli_pt(point);              // The point clicked in doc coords
    ScreenToClient(&cli_pt);
    CPointAp doc_pt = ConvertFromDP(cli_pt);

    if (( (display_.vert_display || display_.char_area) && doc_pt.x >= char_pos(rowsize_)+text_width_ ||
         !(display_.vert_display || display_.char_area) && doc_pt.x >= hex_pos(rowsize_) )  )
    {
        // Don't do anything if off to right
        return;
    }

    if (doc_pt.x < hex_pos(0))
        pCMM->ShowPopupMenu(IDR_CONTEXT_ADDRESS, point.x, point.y, this);
    else if (display_.vert_display)
    {
        // Get selection address
        FILE_ADDRESS start_addr, end_addr;
        GetSelAddr(start_addr, end_addr);

        // Get address clicked on in char area
        FILE_ADDRESS addr = (doc_pt.y/line_height_)*rowsize_ - offset_ +
                            pos_char(doc_pt.x);

        if (addr >= start_addr && addr < end_addr)
            pCMM->ShowPopupMenu(IDR_CONTEXT_SELECTION, point.x, point.y, this);
        else if (!display_.hide_bookmarks &&
                 std::find(GetDocument()->bm_posn_.begin(), GetDocument()->bm_posn_.end(), addr) != GetDocument()->bm_posn_.end())
            pCMM->ShowPopupMenu(IDR_CONTEXT_BOOKMARKS, point.x, point.y, this);
        else if (!display_.hide_highlight && hl_set_.find(addr) != hl_set_.end())
            pCMM->ShowPopupMenu(IDR_CONTEXT_HIGHLIGHT, point.x, point.y, this);
        else if (pos2row(doc_pt) == 0)
            pCMM->ShowPopupMenu(IDR_CONTEXT_CHAR, point.x, point.y, this);
        else
            pCMM->ShowPopupMenu(IDR_CONTEXT_HEX, point.x, point.y, this);
    }
    else if (!display_.char_area || doc_pt.x < char_pos(0))
    {
        // Get selection address
        FILE_ADDRESS start_addr, end_addr;
        GetSelAddr(start_addr, end_addr);

        // Get address clicked on in hex area
        FILE_ADDRESS addr = (doc_pt.y/text_height_)*rowsize_ - offset_ +
                            pos_hex(doc_pt.x);

        if (addr >= start_addr && addr < end_addr)
            pCMM->ShowPopupMenu(IDR_CONTEXT_SELECTION, point.x, point.y, this);
        else if (!display_.hide_bookmarks &&
                 std::find(GetDocument()->bm_posn_.begin(), GetDocument()->bm_posn_.end(), addr) != GetDocument()->bm_posn_.end())
            pCMM->ShowPopupMenu(IDR_CONTEXT_BOOKMARKS, point.x, point.y, this);
        else if (!display_.hide_highlight && hl_set_.find(addr) != hl_set_.end())
            pCMM->ShowPopupMenu(IDR_CONTEXT_HIGHLIGHT, point.x, point.y, this);
        else
            pCMM->ShowPopupMenu(IDR_CONTEXT_HEX, point.x, point.y, this);
    }
    else
    {
        // Get selection address
        FILE_ADDRESS start_addr, end_addr;
        GetSelAddr(start_addr, end_addr);

        // Get address clicked on in char area
        FILE_ADDRESS addr = (doc_pt.y/text_height_)*rowsize_ - offset_ +
                            pos_char(doc_pt.x);

        if (addr >= start_addr && addr < end_addr)
            pCMM->ShowPopupMenu(IDR_CONTEXT_SELECTION, point.x, point.y, this);
        else if (!display_.hide_bookmarks &&
                 std::find(GetDocument()->bm_posn_.begin(), GetDocument()->bm_posn_.end(), addr) != GetDocument()->bm_posn_.end())
            pCMM->ShowPopupMenu(IDR_CONTEXT_BOOKMARKS, point.x, point.y, this);
        else if (!display_.hide_highlight && hl_set_.find(addr) != hl_set_.end())
            pCMM->ShowPopupMenu(IDR_CONTEXT_HIGHLIGHT, point.x, point.y, this);
        else
            pCMM->ShowPopupMenu(IDR_CONTEXT_CHAR, point.x, point.y, this);
    }
#endif
}

/////////////////////////////////////////////////////////////////////////////
// CHexEditView command handlers

void CHexEditView::OnRedraw() 
{
    CRect cli;

    // Work out window height in logical units
    GetClientRect(&cli);
    CRectAp doc_rect = ConvertFromDP(cli);

    // Move display so that caret is centred vertically
    CPointAp pp = GetCaret();
    pp.x = 0;                       // Scroll to left side (but see DisplayCaret() below)
    if ((pp.y -= (doc_rect.bottom-doc_rect.top)/2) < 0)
        pp.y = 0;
    SetScroll(pp);

    DoInvalidate();                 // Also redraw display
    DisplayCaret();                 // Make sure caret not off right side

    ((CHexEditApp *)AfxGetApp())->SaveToMacro(km_redraw);
}

void CHexEditView::OnScrollDown() 
{
    CPointAp pp = GetScroll();
    pp.y += text_height_;
    SetScroll(pp);
    ((CHexEditApp *)AfxGetApp())->SaveToMacro(km_scroll_down);
}

void CHexEditView::OnScrollUp() 
{
    CPointAp pp = GetScroll();
    pp.y -= text_height_;
    SetScroll(pp);
    ((CHexEditApp *)AfxGetApp())->SaveToMacro(km_scroll_up);
}

// Move current position to start of display line
void CHexEditView::OnStartLine() 
{
    num_entered_ = num_del_ = num_bs_ = 0;  // Turn off any consec edits
    FILE_ADDRESS start_addr, end_addr;
    GetSelAddr(start_addr, end_addr);
    int row = 0;
    if (start_addr == end_addr && display_.vert_display)
        row = pos2row(GetCaret());

    // Check if offset has actually changed
    if (real_offset_ != (rowsize_ - start_addr%rowsize_)%rowsize_)
    {
        undo_.push_back(view_undo(undo_offset));
        undo_.back().rowsize = real_offset_;        // Save previous offset for undo
        real_offset_ = offset_ = int((rowsize_ - start_addr%rowsize_)%rowsize_);
        SetSel(addr2pos(start_addr, row), addr2pos(end_addr, row));
        recalc_display();
        DoInvalidate();
        ((CHexEditApp *)AfxGetApp())->SaveToMacro(km_start_line);
    }
    else
    {
        ((CMainFrame *)AfxGetMainWnd())->StatusBarText("Warning: offset not changed");
        ((CHexEditApp *)AfxGetApp())->mac_error_ = 1;
    }
}

void CHexEditView::OnSwap() 
{
    if (display_.vert_display)
        return;

    CHECK_SECURITY(32);

    if (display_.char_area && display_.hex_area)
    {
        num_entered_ = num_del_ = num_bs_ = 0;  // Turn off any consec edits

        // Swap between hex and char areas
        FILE_ADDRESS start_addr, end_addr;
        BOOL end_base = GetSelAddr(start_addr, end_addr);
        undo_.push_back(view_undo(undo_state));
        undo_.back().disp_state = disp_state_;
        display_.edit_char = !display_.edit_char;
        if (display_.edit_char)
            SetHorzBufferZone(1);
        else
            SetHorzBufferZone(2);
        if (end_base)
            SetSel(addr2pos(end_addr), addr2pos(start_addr), true);
        else
            SetSel(addr2pos(start_addr), addr2pos(end_addr));
        DisplayCaret();

        ((CHexEditApp *)AfxGetApp())->SaveToMacro(km_swap_areas);
    }
    else
    {
        ::HMessageBox("Cannot swap to character area - not displayed");
        ((CHexEditApp *)AfxGetApp())->mac_error_ = 5;
    }
}

void CHexEditView::OnUpdateSwap(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(display_.char_area && display_.hex_area);      // disallow swap if both areas not displayed
}

void CHexEditView::OnDel() 
{
    num_entered_ = 0;           // Not entering but deleting

    CPointAp pt_start, pt_end;
    if (GetSel(pt_start, pt_end))
        SetSel(pt_end, pt_start, true);
    else
        SetSel(pt_start, pt_end);     // Calls ValidateCaret - causing caret selection to be moved to valid locations

    if (check_ro("delete"))
        return;

    if (display_.overtype)
    {
		// xxx direct warning if GetDocument()->IsDevice()?
        if (::HMessageBox("You can't delete while in overtype mode.\r"
                          "Do you want to turn off overtype mode?",
                          MB_OKCANCEL) == IDCANCEL)
            return;
        else if (!do_insert())
			return;
    }

    // Handle deletion of chars at caret or selection
    FILE_ADDRESS start, end;
    GetSelAddr(start, end);
//      address = pos2addr(GetCaret());

    CHECK_SECURITY(190);

    if (start == end)
    {
//        if ((long)nRepCnt > GetDocument()->length() - start)
//            nRepCnt = (UINT)(GetDocument()->length() - start);
//        if (nRepCnt > 0)
//            GetDocument()->Change(mod_delforw, start, nRepCnt,
//                                  NULL, num_del_, this);
//        num_del_ += nRepCnt*2;
        if (start >= GetDocument()->length())
        {
            ((CHexEditApp *)AfxGetApp())->mac_error_ = 2;
            return;
        }

        GetDocument()->Change(mod_delforw, start, 1, NULL, num_del_, this);
        num_del_ += 2;
    }
    else
    {
        GetDocument()->Change(mod_delforw, start, end-start,
                                  NULL, 0, this);
        num_del_ = 0;               // Make sure this is separate deletion
    }
    DisplayCaret();                 // Make sure caret is visible

    ((CHexEditApp *)AfxGetApp())->SaveToMacro(km_del);
}

void CHexEditView::OnSelectAll() 
{
    num_entered_ = num_del_ = num_bs_ = 0;      // Can't be editing while mousing
    GetSelAddr(prev_start_, prev_end_);
    prev_row_ = 0;
    if (prev_start_ == prev_end_ && display_.vert_display)
        prev_row_ = pos2row(GetCaret());
    SetSel(addr2pos(0), addr2pos(GetDocument()->length()));
    show_prop();
    show_calc();
    show_pos();

    // Save in undo array if position moved
    if (prev_start_ != 0 || prev_end_ != GetDocument()->length())
    {
        // Save the caret position (or start of selection)
        undo_.push_back(view_undo(undo_move));
        undo_.back().address = prev_start_ | (FILE_ADDRESS(prev_row_)<<62);
        if (prev_start_ != prev_end_)
        {
            // Save the previous selection
            undo_.push_back(view_undo(undo_sel));
            undo_.back().address = prev_end_;
        }

		nav_save(0, GetDocument()->length(), "Select All ");
    }
    ((CHexEditApp *)AfxGetApp())->SaveToMacro(km_sel_all);
}

void CHexEditView::OnSelectLine()
{
    num_entered_ = num_del_ = num_bs_ = 0;
    GetSelAddr(prev_start_, prev_end_);
    prev_row_ = 0;
    if (prev_start_ == prev_end_ && display_.vert_display)
        prev_row_ = pos2row(GetCaret());

    FILE_ADDRESS new_start = ((prev_start_+offset_)/rowsize_) * rowsize_ - offset_;
    FILE_ADDRESS new_end =   ((prev_end_+offset_)/rowsize_ + 1) * rowsize_ - offset_;
    ASSERT(new_end > new_start);
    SetSel(addr2pos(new_start), addr2pos(new_end));
    show_prop();
    show_calc();
    show_pos();

    // Save in undo array if position moved
    if (prev_start_ != new_start || prev_end_ != new_end)
    {
        // Save the caret position (or start of selection)
        undo_.push_back(view_undo(undo_move));
        undo_.back().address = prev_start_ | (FILE_ADDRESS(prev_row_)<<62);
        if (prev_start_ != prev_end_)
        {
            // Save the previous selection
            undo_.push_back(view_undo(undo_sel));
            undo_.back().address = prev_end_;
        }
		nav_save(new_start, new_end, "Select Line");
    }
    ((CHexEditApp *)AfxGetApp())->SaveToMacro(km_sel_line);
}

void CHexEditView::OnInsertBlock()
{
    CNewFile dlg(this, true);

    if (dlg.DoModal() == IDOK)
    {
        const char *data_str = NULL;
        dlg.fill_.create_on_disk = 0;   // Make sure we are not creating a new file
        switch (dlg.fill_.type)
        {
        default:
            ASSERT(0);
            // fall through
        case CNewFile::FILL_HEX:   // count hex digits
            data_str = dlg.fill_hex_value_;
            break;
        case CNewFile::FILL_STRING:   // string
            data_str = dlg.fill_string_value_;
            break;
        case CNewFile::FILL_CLIPBOARD:   // clipboard
            break;
        case CNewFile::FILL_RANDOM:   // random - default to one byte
            data_str = dlg.fill_random_range_;
            break;
        case CNewFile::FILL_NUMBER:   // number - get size from type
            data_str = dlg.fill_number_value_;
            break;
        }

        do_insert_block(dlg.fill_state_, data_str);
    }
    theApp.CheckBGSearchFinished();
}

void CHexEditView::do_insert_block(_int64 params, const char *data_str)
{
    // Can't modify the file if view is read only or in overtype mode
    if (check_ro("insert block"))
        return;

    if (display_.overtype)
    {
		// xxx direct warning if GetDocument()->IsDevice()?
        if (::HMessageBox("You can't insert a block while in overtype mode.\r"
                          "Do you want to turn off overtype mode?",
                          MB_OKCANCEL) == IDCANCEL)
        {
            theApp.mac_error_ = 5;
            return;
        }
        else if (!do_insert())
			return;
   }

    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing


    FILE_ADDRESS addr = GetPos();                       // Current caret (address to insert file)
    FILE_ADDRESS len = GetDocument()->insert_block(addr, params, data_str, this);
    if (len > -1)
    {
        SetSel(addr2pos(addr), addr2pos(addr+len));
        DisplayCaret();
        theApp.SaveToMacro(km_insert, params);
        theApp.SaveToMacro(km_insert_str, data_str);
    }
}

void CHexEditView::OnUpdateInsertBlock(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(!GetDocument()->read_only()); // disallow insert if file is read only
}

// Get file name, insert into document and select inserted bytes
void CHexEditView::OnReadFile() 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());

    // Get name of file to read
    CHexFileDialog dlgFile("ReadFileDlg", TRUE, NULL, aa->current_read_,
                           OFN_HIDEREADONLY | OFN_SHOWHELP | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT,
                           aa->GetCurrentFilters(), this);

    // Set up the title of the dialog
    dlgFile.m_ofn.lpstrTitle = "Insert File";

    if (dlgFile.DoModal() == IDOK)
    {
        aa->current_read_ = dlgFile.GetPathName();

        CHECK_SECURITY(23);

        do_read(aa->current_read_);
    }

    // BG search finished message may be lost if modeless dlg running
    aa->CheckBGSearchFinished();
}

// Actually open and read the file and select the inserted bytes
void CHexEditView::do_read(CString file_name)
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
    // Can't modify the file if view is read only or in overtype mode

    if (check_ro("insert file"))
        return;

    if (display_.overtype)
    {
		// xxx direct warning if GetDocument()->IsDevice()?
        if (::HMessageBox("You can't insert a file while in overtype mode.\r"
                          "Do you want to turn off overtype mode?",
                          MB_OKCANCEL) == IDCANCEL)
        {
            aa->mac_error_ = 5;
            return;
        }
        else if (!do_insert())
			return;
    }

    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    FILE_ADDRESS addr = GetPos();                       // Current caret (address to insert file)
    FILE_ADDRESS data_len = CFile64::GetSize(file_name);
    if (data_len == 0)
    {
        // No need to insert a zero length file
        ((CMainFrame *)AfxGetMainWnd())->StatusBarText("File is empty: nothing inserted");
        aa->mac_error_ = 1;
        return;
    }

    int idx = -1;                       // Index into docs data_file_ array (or -1 if no slots avail.)
    if (data_len > (16*1024*1024) && (idx = GetDocument()->AddDataFile(file_name)) != -1)
    {
        // Use the file in situ
        GetDocument()->Change(mod_insert_file, addr, data_len, NULL, idx, this);
    }
    else
    {
		if (data_len > UINT_MAX)  // why is there no SIZE_T_MAX?
		{
			::HMessageBox("HexEdit is out of temporary files \n"
                          "and cannot open such a large file. \n"
                          "Please save the file to free \n"
						  "temporary file handles and try again.\n",
						  MB_OK);
			return;
		}

        if (data_len > 128*1024*1024)  // 128 Mb file may be too big
        {
			if (::HMessageBox("WARNING: HexEdit is out of temporary file \n"
                              "handles and opening such a large file may \n"
                              "cause memory exhaustion.  Please click NO \n"
                              "and save the file to free handles. \n"
							  "Or, click YES to continue.\n\n"
                              "Do you want to continue?", MB_YESNO) != IDYES)
                return;
        }

        // Open the file for reading
        CFile ff;                   // Don't make this CFile64 as we don't want to read > 2Gbytes into memory
        CFileException fe;          // Used to receive file error information

        if (!ff.Open(file_name,
            CFile::modeRead|CFile::shareDenyWrite|CFile::typeBinary, &fe) )
        {
            // Display info about why the open failed
            CString mess(file_name);
            CFileStatus fs;

            switch (fe.m_cause)
            {
            case CFileException::fileNotFound:
                mess += "\rdoes not exist";
                break;
            case CFileException::badPath:
                mess += "\ris an invalid file name or the drive/path does not exist";
                break;
            case CFileException::tooManyOpenFiles:
                mess += "\r- too many files already open";
                break;
            case CFileException::accessDenied:
                if (!CFile::GetStatus(file_name, fs))
                    mess += "\rdoes not exist";
                else
                {
                    if (fs.m_attribute & CFile::directory)
                        mess += "\ris a directory";
                    else if (fs.m_attribute & (CFile::volume|CFile::hidden|CFile::system))
                        mess += "\ris a special file";
                    else
                        mess += "\rcannot be used (reason unknown)";
                }
                break;
            case CFileException::sharingViolation:
            case CFileException::lockViolation:
                mess += "\ris in use";
                break;
            case CFileException::hardIO:
                mess += "\r- hardware error";
                break;
            default:
                mess += "\rcould not be opened (reason unknown)";
                break;
            }
            ::HMessageBox(mess);

            aa->mac_error_ = 10;
            return;
        }

        CWaitCursor wait;                           // Turn on wait cursor (hourglass)

        // If the user tries to insert a huge file we might run out of memory so catch this
        try
        {
            // Get memory to read all of the file
            unsigned char *file_data = new unsigned char[size_t(data_len)];

            // Read the file into memory
            if (ff.Read((void *)file_data, (UINT)data_len) != data_len)
            {
                ::HMessageBox("Not all of the file could be read");
                aa->mac_error_ = 10;
                return;
            }
            ff.Close();

            // Make the memory part of the document
            GetDocument()->Change(mod_insert, addr, data_len, file_data, 0, this);
            delete[] file_data;
        }
        catch(std::bad_alloc)
        {
            ::HMessageBox("Insufficient memory to read the file");
            aa->mac_error_ = 10;
            return;
        }
    }
    ASSERT(data_len > 0);
    SetSel(addr2pos(addr), addr2pos(addr+data_len));
    DisplayCaret();
    aa->SaveToMacro(km_read_file, file_name);
}

void CHexEditView::OnUpdateReadFile(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(!GetDocument()->read_only()); // disallow insert if file is read only
}

void CHexEditView::OnEditWriteFile() 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    // Get the selection
    FILE_ADDRESS start, end;
    GetSelAddr(start, end);
    ASSERT(start >= 0 && start <= end && end <= GetDocument()->length());
    if (start == end)
    {
        // Nothing selected, presumably in macro playback
        ASSERT(aa->playing_);
        ::HMessageBox("Nothing selected to write to file!");
        aa->mac_error_ = 10;
        return;
    }

    // Get the file name to write the selection to
    CHexFileDialog dlgFile("WriteFileDlg", FALSE, NULL, aa->current_write_,
                           OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_SHOWHELP | OFN_NOCHANGEDIR,
                           aa->GetCurrentFilters(), this);

    // Set up the title of the dialog
    dlgFile.m_ofn.lpstrTitle = "Save Selection As";

//    if (!aa->current_write_.IsEmpty())
//        dlgFile.m_ofn.lpstrInitialDir = aa->current_write_;

    if (dlgFile.DoModal() != IDOK)
    {
        aa->mac_error_ = 2;
        return;
    }

    aa->current_write_ = dlgFile.GetPathName();

    CHECK_SECURITY(25);

    // Write to the file
    CWaitCursor wait;                           // Turn on wait cursor (hourglass)
    if (GetDocument()->WriteData(aa->current_write_, start, end))
        aa->SaveToMacro(km_write_file);
    else
        aa->current_write_.Empty();

    // BG search finished message may be lost if modeless dlg running
    aa->CheckBGSearchFinished();
}

void CHexEditView::OnEditAppendFile() 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    // Get the selection
    FILE_ADDRESS start, end;
    GetSelAddr(start, end);
    ASSERT(start >= 0 && start <= end && end <= GetDocument()->length());
    if (start == end)
    {
        // Nothing selected, presumably in macro playback
        ASSERT(aa->playing_);
        ::HMessageBox("Nothing selected to append to file!");
        aa->mac_error_ = 10;
        return;
    }

    // Get the file name to write the selection to
    CHexFileDialog dlgFile("WriteFileDlg", TRUE, NULL, aa->current_write_,
                        OFN_HIDEREADONLY | OFN_SHOWHELP | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT,
                        aa->GetCurrentFilters(), this);

    // Set up the title of the dialog
    dlgFile.m_ofn.lpstrTitle = "Append Selection To";

    if (dlgFile.DoModal() != IDOK)
    {
        aa->mac_error_ = 2;
        return;
    }

    CHECK_SECURITY(15);

    aa->current_write_ = dlgFile.GetPathName();

    // Write to the file
    CWaitCursor wait;                           // Turn on wait cursor (hourglass)
    if (GetDocument()->WriteData(aa->current_write_, start, end, TRUE))
        aa->SaveToMacro(km_append_file);
    else
        aa->current_write_.Empty();

    // BG search finished message may be lost if modeless dlg running
    aa->CheckBGSearchFinished();
}

void CHexEditView::OnEditAppendSameFile() 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    // Get the selection
    FILE_ADDRESS start, end;
    GetSelAddr(start, end);
    ASSERT(start >= 0 && start <= end && end <= GetDocument()->length());
    CHECK_SECURITY(25);

    if (start == end)
    {
        // Nothing selected, presumably in macro playback
        ASSERT(aa->playing_);
        ::HMessageBox("Nothing selected to append to file!");
        aa->mac_error_ = 10;
        return;
    }

    // Write to the file
    CWaitCursor wait;                           // Turn on wait cursor (hourglass)
    if (GetDocument()->WriteData(aa->current_write_, start, end, TRUE))
        aa->SaveToMacro(km_append_same_file);
    else
        aa->current_write_.Empty();

    // BG search finished message may be lost if modeless dlg running
    aa->CheckBGSearchFinished();
}

void CHexEditView::OnUpdateEditAppendSameFile(CCmdUI* pCmdUI) 
{
    if (theApp.current_write_.IsEmpty())
        pCmdUI->Enable(FALSE);
    else
        OnUpdateClipboard(pCmdUI);
}

void CHexEditView::OnExportSRecord(UINT nID) 
{
    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    // Get the selection
    FILE_ADDRESS start, end;
    GetSelAddr(start, end);
    ASSERT(start >= 0 && start <= end && end <= GetDocument()->length());
    if (start == end)
    {
        // Nothing selected, presumably in macro playback
        ASSERT(theApp.playing_);
        ::HMessageBox("Nothing selected to export!");
        theApp.mac_error_ = 10;
        return;
    }

    FILE_ADDRESS last_S_address;        // Byte past the last address to be written out
    int stype = 3;                          // Type of S records to write (1, 2, or 3)
    if (theApp.export_base_addr_ < 0)
        last_S_address = end;
    else
        last_S_address = end - start + theApp.export_base_addr_;

    // Check that end address is not too big for S Record
    if (nID == ID_EXPORT_S1)
    {
        // Addresses must be within 16 bit range
        if (last_S_address > 0x10000)
        {
            ASSERT(theApp.playing_);
            ::HMessageBox("End address too big for S1 export!");
            theApp.mac_error_ = 10;
            return;
        }
        stype = 1;
    }
    else if (nID == ID_EXPORT_S2)
    {
        // Addresses must be within 24 bit range
        if (last_S_address > 0x1000000)
        {
            ASSERT(theApp.playing_);
            ::HMessageBox("End address too big for S2 export!");
            theApp.mac_error_ = 10;
            return;
        }
        stype = 2;
    }
    else if (nID == ID_EXPORT_S3)
    {
        // Addresses must be within 32 bit range
        if (last_S_address > 0x100000000)
        {
            ASSERT(theApp.playing_);
            ::HMessageBox("End address too big for S3 export!");
            theApp.mac_error_ = 10;
            return;
        }
        ASSERT(stype == 3);
    }

    // Get the file name to write the selection to
    CHexFileDialog dlgFile("ExportFileDlg", FALSE, NULL, theApp.current_export_,
                        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_SHOWHELP | OFN_NOCHANGEDIR,
                        theApp.GetCurrentFilters(), this);

    // Set up the title of the dialog
    dlgFile.m_ofn.lpstrTitle = "Export S Records";

    if (dlgFile.DoModal() != IDOK)
    {
        theApp.mac_error_ = 2;
        return;
    }

    CHECK_SECURITY(25);

    theApp.current_export_ = dlgFile.GetPathName();

    // Write to the file
    CWaitCursor wait;                           // Turn on wait cursor (hourglass)
    CWriteSRecord wsr(theApp.current_export_,
                      theApp.export_base_addr_ >= 0 ? theApp.export_base_addr_ : long(start),
                      stype,
                      theApp.export_line_len_);
    if (!wsr.Error().IsEmpty())
    {
        ::HMessageBox(wsr.Error());
        theApp.mac_error_ = 10;
        theApp.current_export_.Empty();
    }
    else
    {
        unsigned char *buf = new unsigned char[4096];
        size_t len;
        for (FILE_ADDRESS curr = start; curr < end; curr += len)
        {
            len = min(4096, int(end - curr));
            VERIFY(GetDocument()->GetData(buf, len, curr) == len);

            wsr.Put(buf, len);
            if (!wsr.Error().IsEmpty())
            {
                ::HMessageBox(wsr.Error());
                theApp.mac_error_ = 10;
                theApp.current_export_.Empty();
                break;
            }
        }
        delete[] buf;
        ASSERT(curr == end);

        if (theApp.mac_error_ < 10)
            theApp.SaveToMacro(km_write_file, stype);  // stype == 1,2, or 3
    }

    // BG search finished message may be lost if modeless dlg running
    theApp.CheckBGSearchFinished();
}

void CHexEditView::OnUpdateExportSRecord(CCmdUI* pCmdUI) 
{
    FILE_ADDRESS start, end;
    GetSelAddr(start, end);
    ASSERT(start >= 0 && start <= end && end <= GetDocument()->length());

    if (start >= end)
    {
        // We can't export if there is no selection at all
        pCmdUI->Enable(FALSE);
        return;
    }

    // Work out the last address that needs to be exported
    if (theApp.export_base_addr_ >= 0)
        end = end - start + theApp.export_base_addr_;

    if (pCmdUI->m_nID == ID_EXPORT_S1)
    {
        // Addresses must be within 16 bit range
        pCmdUI->Enable(end <= 0x10000);
    }
    else if (pCmdUI->m_nID == ID_EXPORT_S2)
    {
        // Addresses must be within 24 bit range
        pCmdUI->Enable(end <= 0x1000000);
    }
    else if (pCmdUI->m_nID == ID_EXPORT_S3)
    {
        // Addresses must be within 32 bit range
        pCmdUI->Enable(end <= 0x100000000);
    }
}

void CHexEditView::OnImportMotorolaS()
{
    // Get name of file to import
//    CFileDialog dlgFile(TRUE, NULL, theApp.current_import_,
//                        OFN_HIDEREADONLY | OFN_SHOWHELP | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR,
//                        theApp.GetCurrentFilters(), this);
    CImportDialog dlgFile(theApp.current_import_,
                        OFN_HIDEREADONLY | OFN_SHOWHELP | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT,
                        "Motorola S Files (*.s)|*.s|"+theApp.GetCurrentFilters(), this);

    // Set up the title of the dialog
    dlgFile.m_ofn.lpstrTitle = "Import Motorola S Records";

    if (dlgFile.DoModal() == IDOK)
    {
        theApp.current_import_ = dlgFile.GetPathName();

        do_motorola(theApp.current_import_);
    }

    // BG search finished message may be lost if modeless dlg running
    theApp.CheckBGSearchFinished();
}

void CHexEditView::do_motorola(CString file_name)
{
    if (check_ro("import file"))
        return;

    if (!theApp.import_discon_ && display_.overtype)
    {
        if (::HMessageBox("You can't import (contiguous) while in overtype mode.\r"
                          "Do you want to turn off overtype mode?",
                          MB_OKCANCEL) == IDCANCEL)
        {
            theApp.mac_error_ = 5;
            return;
        }
        else if (!do_insert())
			return;
    }

    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    CWaitCursor wait;                           // Turn on wait cursor (hourglass)
    CHECK_SECURITY(33);

    if (theApp.import_discon_)
    {
        BOOL ptoo = FALSE;              // First change is new undo op, thence merged with it
        if (theApp.import_highlight_ && !hl_set_.empty())
        {
            undo_.push_back(view_undo(undo_highlight));
            undo_.back().phl = new range_set<FILE_ADDRESS>(hl_set_);
            hl_set_.clear();
            DoInvalidate();
            ptoo = TRUE;
        }

        CReadSRecord rsr(file_name, TRUE);

        if (!rsr.Error().IsEmpty())
        {
            ::HMessageBox(rsr.Error());
            theApp.mac_error_ = 10;
            return;
        }

        // Read 
        int count, skipped;             // Total records read and ignored
        size_t data_len = 1024;
        unsigned char *file_data = new unsigned char[data_len];  // one S record
        unsigned long address;
        int len;                        // length of data from one S record

        for (count = skipped = 0; (len = rsr.Get(file_data, data_len, address)) > 0; ++count)
        {
            if (address + len > GetDocument()->length())
            {
                ++skipped;
                continue;
            }
            if (theApp.import_highlight_)
            {
                // Highlight the record
                add_highlight(address, address+len, ptoo);
                ptoo = TRUE;
            }
            GetDocument()->Change(mod_replace, FILE_ADDRESS(address), FILE_ADDRESS(len), file_data, 0, this, ptoo);
            ptoo = TRUE;  // force subsequent changes to be merged into same undo operation
        }
        delete[] file_data;

        if (!rsr.Error().IsEmpty())
            ::HMessageBox(rsr.Error());
        CString mess;
        if (skipped > 0)
            mess.Format("Wrote %ld records\r\nIgnored %ld records\r\nimporting \"%s\"",
                        long(count - skipped),
                        long(skipped),
                        file_name);
        else
            mess.Format("Wrote %ld records importing \"%s\"", long(count), file_name);
        AfxMessageBox(mess);

        theApp.SaveToMacro(km_import_motorola, file_name);
    }
    else
    {
        CReadSRecord rsr(file_name);

        if (!rsr.Error().IsEmpty())
        {
            ::HMessageBox(rsr.Error());
            theApp.mac_error_ = 10;
            return;
        }

        // We need to store the data in fairly large chunks otherwise the doc undo
        // stack becomes very full and slows things down
        size_t data_len = 32768;
        unsigned char *file_data = new unsigned char[data_len];  // work buffer
        unsigned char *pp = file_data;  // Ptr into file_data where we are currently storing
        unsigned long address;          // Import S record address (ignored)
        int len;                        // length of data from one S record
        bool first(true);               // is this the first S record read from the file
        FILE_ADDRESS start_addr, end_addr, curr_addr = -1;

        GetSelAddr(start_addr, end_addr);  // Get the current selection

        for (;;)
        {
            len = rsr.Get(pp, data_len - (pp - file_data), address);

            if (len <= 0 || (pp - file_data) + len > (int(data_len) - 256))
            {
                // The buffer is almost full or we're finished reading the file - so save buffer
                if (first)
                {
                    curr_addr = start_addr;
                    if (start_addr < end_addr)
                    {
                        // Delete current selection and insert new data
                        GetDocument()->Change(mod_delforw, start_addr, end_addr-start_addr, NULL, 0, this);
                        GetDocument()->Change(mod_insert, curr_addr, pp - file_data + len, file_data, 0, this, TRUE);
                    }
                    else
                        GetDocument()->Change(mod_insert, curr_addr, pp - file_data + len, file_data, 0, this);
                    first = false;
                }
                else
                {
                    ASSERT(curr_addr != -1);
                    GetDocument()->Change(mod_insert, curr_addr, pp - file_data + len, file_data, 0, this, TRUE);
                }
                curr_addr += pp - file_data + len;
                pp = file_data;
            }
            else
                pp += len;

            if (len <= 0)
                break;
        }
        delete[] file_data;

        if (!rsr.Error().IsEmpty())
            ::HMessageBox(rsr.Error());

        theApp.SaveToMacro(km_import_motorola, file_name);

        SetSel(addr2pos(start_addr), addr2pos(curr_addr));
        DisplayCaret();
    }
}

void CHexEditView::OnUpdateImportMotorolaS(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!GetDocument()->read_only()); // disallow insert if file is read only
}

void CHexEditView::OnImportIntel() 
{
    // Get name of file to import
//    CFileDialog dlgFile(TRUE, NULL, theApp.current_import_,
//                        OFN_HIDEREADONLY | OFN_SHOWHELP | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR,
//                        theApp.GetCurrentFilters(), this);
    CImportDialog dlgFile(theApp.current_import_,
                        OFN_HIDEREADONLY | OFN_SHOWHELP | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT,
                        "Intel Hex Files (*,hex, *.ihx)|*.hex;*.ihx|"+theApp.GetCurrentFilters(), this);

    // Set up the title of the dialog
    dlgFile.m_ofn.lpstrTitle = "Import Intel Hex File";

    if (dlgFile.DoModal() == IDOK)
    {
        theApp.current_import_ = dlgFile.GetPathName();

        do_intel(theApp.current_import_);
    }

    // BG search finished message may be lost if modeless dlg running
    theApp.CheckBGSearchFinished();
}

void CHexEditView::do_intel(CString file_name)
{
    if (check_ro("import Intel hex file"))
        return;

    if (!theApp.import_discon_ && display_.overtype)
    {
        if (::HMessageBox("You can't import (contiguous) while in overtype mode.\r"
                          "Do you want to turn off overtype mode?",
                          MB_OKCANCEL) == IDCANCEL)
        {
            theApp.mac_error_ = 5;
            return;
        }
        else if (!do_insert())
			return;
    }

    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    CHECK_SECURITY(32);
    CWaitCursor wait;                           // Turn on wait cursor (hourglass)

    if (theApp.import_discon_)
    {
        BOOL ptoo = FALSE;              // First change is new undo op, thence merged with it
        if (theApp.import_highlight_ && !hl_set_.empty())
        {
            undo_.push_back(view_undo(undo_highlight));
            undo_.back().phl = new range_set<FILE_ADDRESS>(hl_set_);
            hl_set_.clear();
            DoInvalidate();
            ptoo = TRUE;
        }

        CReadIntelHex rih(file_name, TRUE);

        if (!rih.Error().IsEmpty())
        {
            ::HMessageBox(rih.Error());
            theApp.mac_error_ = 10;
            return;
        }

        // Read 
        int count, skipped;             // Total records read and ignored
        size_t data_len = 1024;
        unsigned char *file_data = new unsigned char[data_len];  // one S record
        unsigned long address;
        int len;                        // length of data from one S record

        for (count = skipped = 0; (len = rih.Get(file_data, data_len, address)) > 0; ++count)
        {
            if (address + len > GetDocument()->length())
            {
                ++skipped;
                continue;
            }
            if (theApp.import_highlight_)
            {
                // Highlight the record
                add_highlight(address, address+len, ptoo);
                ptoo = TRUE;
            }
            GetDocument()->Change(mod_replace, FILE_ADDRESS(address), FILE_ADDRESS(len), file_data, 0, this, ptoo);
            ptoo = TRUE;  // force subsequent changes to be merged into same undo operation
        }
        delete[] file_data;

        if (!rih.Error().IsEmpty())
            ::HMessageBox(rih.Error());
        CString mess;
        if (skipped > 0)
            mess.Format("Wrote %ld records\r\nIgnored %ld records\r\nimporting \"%s\"",
                        long(count - skipped),
                        long(skipped),
                        file_name);
        else
            mess.Format("Wrote %ld records importing \"%s\"", long(count), file_name);
        AfxMessageBox(mess);

        theApp.SaveToMacro(km_import_intel, file_name);
    }
    else
    {
        CReadIntelHex rih(file_name);

        if (!rih.Error().IsEmpty())
        {
            ::HMessageBox(rih.Error());
            theApp.mac_error_ = 10;
            return;
        }

        // We need to store the data in fairly large chunks otherwise the doc undo
        // stack becomes very full and slows things down
        size_t data_len = 32768;
        unsigned char *file_data = new unsigned char[data_len];  // work buffer
        unsigned char *pp = file_data;  // Ptr into file_data where we are currently storing
        int len;                        // length of data from one S record
        unsigned long address;          // Import record address (ignored)
        bool first(true);               // is this the first S record read from the file
        FILE_ADDRESS start_addr, end_addr, curr_addr = -1;

        GetSelAddr(start_addr, end_addr);  // Get the current selection

        for (;;)
        {
            len = rih.Get(pp, data_len - (pp - file_data), address);

            if (len <= 0 || (pp - file_data) + len > (int(data_len) - 256))
            {
                // The buffer is almost full or we're finished reading the file - so save buffer
                if (first)
                {
                    curr_addr = start_addr;
                    if (start_addr < end_addr)
                    {
                        // Delete current selection and insert new data
                        GetDocument()->Change(mod_delforw, start_addr, end_addr-start_addr, NULL, 0, this);
                        GetDocument()->Change(mod_insert, curr_addr, pp - file_data + len, file_data, 0, this, TRUE);
                    }
                    else
                        GetDocument()->Change(mod_insert, curr_addr, pp - file_data + len, file_data, 0, this);
                    first = false;
                }
                else
                {
                    ASSERT(curr_addr != -1);
                    GetDocument()->Change(mod_insert, curr_addr, pp - file_data + len, file_data, 0, this, TRUE);
                }
                curr_addr += pp - file_data + len;
                pp = file_data;
            }
            else
                pp += len;

            if (len <= 0)
                break;
        }
        delete[] file_data;

        if (!rih.Error().IsEmpty())
            ::HMessageBox(rih.Error());

        theApp.SaveToMacro(km_import_intel, file_name);

        SetSel(addr2pos(start_addr), addr2pos(curr_addr));
        DisplayCaret();
    }
}

void CHexEditView::OnUpdateImportIntel(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(!GetDocument()->read_only()); // disallow insert if file is read only
}

void CHexEditView::OnExportIntel() 
{
    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    // Get the selection
    FILE_ADDRESS start, end;
    GetSelAddr(start, end);
    ASSERT(start >= 0 && start <= end && end <= GetDocument()->length());
    if (start == end)
    {
        // Nothing selected, presumably in macro playback
        ASSERT(theApp.playing_);
        ::HMessageBox("Nothing selected to export!");
        theApp.mac_error_ = 10;
        return;
    }

    FILE_ADDRESS last_address;        // Byte past the last address to be written out
    if (theApp.export_base_addr_ < 0)
        last_address = end;
    else
        last_address = end - start + theApp.export_base_addr_;

    // Check that end address is not too big for Intel Hex Record
    // Addresses must be within 16 bit range
    if (last_address > 0x10000)
    {
        ASSERT(theApp.playing_);
        ::HMessageBox("End address too big for Intel hex address field!");
        theApp.mac_error_ = 10;
        return;
    }

    // Get the file name to write the selection to
    CHexFileDialog dlgFile("ExportFileDlg", FALSE, NULL, theApp.current_export_,
                        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_SHOWHELP | OFN_NOCHANGEDIR,
                        theApp.GetCurrentFilters(), this);

    // Set up the title of the dialog
    dlgFile.m_ofn.lpstrTitle = "Export Intel Hex";

    if (dlgFile.DoModal() != IDOK)
    {
        theApp.mac_error_ = 2;
        return;
    }

    theApp.current_export_ = dlgFile.GetPathName();

    // Write to the file
    CWaitCursor wait;                           // Turn on wait cursor (hourglass)
    CWriteIntelHex wih(theApp.current_export_,
                      theApp.export_base_addr_ >= 0 ? theApp.export_base_addr_ : long(start), 
                      theApp.export_line_len_);
    if (!wih.Error().IsEmpty())
    {
        ::HMessageBox(wih.Error());
        theApp.mac_error_ = 10;
        theApp.current_export_.Empty();
    }
    else
    {
        unsigned char *buf = new unsigned char[4096];
        size_t len;

        CHECK_SECURITY(16);

        for (FILE_ADDRESS curr = start; curr < end; curr += len)
        {
            len = min(4096, int(end - curr));
            VERIFY(GetDocument()->GetData(buf, len, curr) == len);

            wih.Put(buf, len);
            if (!wih.Error().IsEmpty())
            {
                ::HMessageBox(wih.Error());
                theApp.mac_error_ = 10;
                theApp.current_export_.Empty();
                break;
            }
        }
        delete[] buf;
        ASSERT(curr == end);

        if (theApp.mac_error_ < 10)
            theApp.SaveToMacro(km_write_file, 7);
    }

    // BG search finished message may be lost if modeless dlg running
    theApp.CheckBGSearchFinished();
}

void CHexEditView::OnUpdateExportIntel(CCmdUI* pCmdUI) 
{
    FILE_ADDRESS start, end;
    GetSelAddr(start, end);
    ASSERT(start >= 0 && start <= end && end <= GetDocument()->length());

    if (start >= end)
    {
        // We can't export if there is no selection at all
        pCmdUI->Enable(FALSE);
        return;
    }

    // Work out the last address that needs to be exported
    if (theApp.export_base_addr_ >= 0)
        end = end - start + theApp.export_base_addr_;

    // Addresses must be within 16 bit range
    pCmdUI->Enable(end <= 0x10000);
}

void CHexEditView::OnImportHexText() 
{
    // Get name of file to import
    CHexFileDialog dlgFile("ImportFileDlg", TRUE, NULL, theApp.current_import_,
                        OFN_HIDEREADONLY | OFN_SHOWHELP | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT,
                        theApp.GetCurrentFilters(), this);

    // Set up the title of the dialog
    dlgFile.m_ofn.lpstrTitle = "Import Hex Text File";

    if (dlgFile.DoModal() == IDOK)
    {
        theApp.current_import_ = dlgFile.GetPathName();

        do_hex_text(theApp.current_import_);
    }

    // BG search finished message may be lost if modeless dlg running
    theApp.CheckBGSearchFinished();
}

void CHexEditView::do_hex_text(CString file_name)
{
    if (check_ro("import hex text file"))
        return;

    if (display_.overtype)
    {
        if (::HMessageBox("You can't import while in overtype mode.\r"
                          "Do you want to turn off overtype mode?",
                          MB_OKCANCEL) == IDCANCEL)
        {
            theApp.mac_error_ = 5;
            return;
        }
        else if (!do_insert())
			return;
    }

    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    CHECK_SECURITY(30);
    CWaitCursor wait;                           // Turn on wait cursor (hourglass)

    CStdioFile ff;                      // Text file we are reading from
    CFileException fe;                  // Stores file exception info

    // Open the file
    if (!ff.Open(file_name, CFile::modeRead|CFile::shareDenyWrite|CFile::typeText, &fe))
    {
        ::HMessageBox(::FileErrorMessage(&fe, CFile::modeRead));
        theApp.mac_error_ = 10;
        return;
    }

    // Input buffer (read from hex text file)
    char buffer[520];                   // Where read string is stored
    char *pp;                           // Ptr into read string
    long line_no = 0;                   // Current text line read

    // We need to store the data in fairly large chunks otherwise the doc undo
    // stack becomes very full and slows things down
    size_t data_len = 32768;
    unsigned char *file_data = new unsigned char[data_len];  // work buffer
    unsigned char *pout = file_data;   // Ptr into file_data where we are currently storing

    bool first(true);               // is this the first S record read from the file
    FILE_ADDRESS start_addr, end_addr, curr_addr = -1;

    GetSelAddr(start_addr, end_addr);  // Get the current selection

    buffer[0] = '\0'; pp = buffer;
    for (;;)
    {
        // Skip whitespace/unused characters etc
        while (*pp != '\0' && !isalpha(*pp) && !isdigit(*pp))
            ++pp;

        // If at end of string get the next line from the file
        while (*pp == '\0')
        {
            try
            {
                pp = ff.ReadString(buffer, sizeof(buffer)-1);
                ++line_no;
            }
            catch (CFileException *pfe)
            {
                ::HMessageBox(::FileErrorMessage(pfe, CFile::modeRead));
                pfe->Delete();
                delete[] file_data;
                return;
            }

            if (pp == NULL)
                goto end_file;                  // EOF

            // Skip leading whitespace/unused characters etc
            while (*pp != '\0' && !isalpha(*pp) && !isdigit(*pp))
                ++pp;
        }

        if (!isxdigit(*pp))
        {
            CString ss;
            ss.Format("Unexpected alpha characters in hex text file at line %ld", long(line_no));
            ::HMessageBox(ss);
            theApp.mac_error_ = 5;
            delete[] file_data;
            return;
        }

        if (isdigit(*pp))
            *pout  = *pp - '0';
        else if (isupper(*pp))
            *pout  = *pp - 'A' + 10;
        else
            *pout  = *pp - 'a' + 10;
        ++pp;

        // If pair of hex digits read as 2 nybbles of a byte
        if (isxdigit(*pp))
        {
            if (isdigit(*pp))
                *pout = (*pout << 4) | (*pp - '0');
            else if (isupper(*pp))
                *pout = (*pout << 4) | (*pp - 'A' + 10);
            else
                *pout = (*pout << 4) | (*pp - 'a' + 10);
            ++pp;
        }

        // Check if the output buffer is full
        if (++pout >= file_data + data_len)
        {
            ASSERT(pout == file_data + data_len);

            // The buffer is almost full or we're finished reading the file - so save buffer
            if (first)
            {
                ASSERT(curr_addr == -1);
                curr_addr = start_addr;
                if (start_addr < end_addr)
                {
                    // Delete current selection and insert new data
                    GetDocument()->Change(mod_delforw, start_addr, end_addr-start_addr, NULL, 0, this);
                    GetDocument()->Change(mod_insert, curr_addr, pout - file_data, file_data, 0, this, TRUE);
                }
                else
                    GetDocument()->Change(mod_insert, curr_addr, pout - file_data, file_data, 0, this);
                first = false;
            }
            else
            {
                ASSERT(curr_addr != -1);
                GetDocument()->Change(mod_insert, curr_addr, pout - file_data, file_data, 0, this, TRUE);
            }
            curr_addr += pout - file_data;

            pout = file_data;
        }
    }

end_file:
    // Write out any partial buffer at end
    if (pout > file_data)
    {
        if (first)
        {
            ASSERT(curr_addr == -1);
            curr_addr = start_addr;
            if (start_addr < end_addr)
            {
                // Delete current selection and insert new data
                GetDocument()->Change(mod_delforw, start_addr, end_addr-start_addr, NULL, 0, this);
                GetDocument()->Change(mod_insert, curr_addr, pout - file_data, file_data, 0, this, TRUE);
            }
            else
                GetDocument()->Change(mod_insert, curr_addr, pout - file_data, file_data, 0, this);
            first = false;
        }
        else
        {
            ASSERT(curr_addr != -1);
            GetDocument()->Change(mod_insert, curr_addr, pout - file_data, file_data, 0, this, TRUE);
        }
        curr_addr += pout - file_data;
    }
    delete[] file_data;
    theApp.SaveToMacro(km_import_text, file_name);

    SetSel(addr2pos(start_addr), addr2pos(curr_addr));
    DisplayCaret();
}

void CHexEditView::OnUpdateImportHexText(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(!GetDocument()->read_only()); // disallow insert if file is read only
}

void CHexEditView::OnExportHexText() 
{
    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();
    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    // Get the selection
    FILE_ADDRESS start_addr, end_addr;
    GetSelAddr(start_addr, end_addr);
    ASSERT(start_addr >= 0 && start_addr <= end_addr && end_addr <= GetDocument()->length());
    if (start_addr == end_addr)
    {
        // Nothing selected, presumably in macro playback
        ASSERT(theApp.playing_);
        ::HMessageBox("Nothing selected to export!");
        theApp.mac_error_ = 10;
        return;
    }

    // Get the file name to write the selection to
    CHexFileDialog dlgFile("ExportFileDlg", FALSE, NULL, theApp.current_export_,
                        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_SHOWHELP | OFN_NOCHANGEDIR,
                        theApp.GetCurrentFilters(), this);

    // Set up the title of the dialog
    dlgFile.m_ofn.lpstrTitle = "Export Hex Text";

    if (dlgFile.DoModal() != IDOK)
    {
        theApp.mac_error_ = 2;
        return;
    }

    theApp.current_export_ = dlgFile.GetPathName();

    // Now write to the file
    CWaitCursor wait;                           // Turn on wait cursor (hourglass)
    CHECK_SECURITY(48);

    CFile64 ff;
    CFileException fe;                      // Stores file exception info

    // Open the file
    if (!ff.Open(theApp.current_export_,
                 CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive|CFile::typeBinary,
                 &fe))
    {
        ::HMessageBox(::FileErrorMessage(&fe, CFile::modeWrite));
        theApp.mac_error_ = 10;
        theApp.current_export_.Empty();
		return;
    }

    // This could be perhaps 2 or 3 times faster by buffering more than a line of text at a time

    // Buffer used to hold bits of the binary file to convert
    unsigned char *buf = NULL;  // Buffer to hold some input
    char *out = NULL;           // Buffer for output of one line of text
    unsigned char *pin;
    FILE_ADDRESS curr;
	size_t len;

	try
	{
		buf = new unsigned char[theApp.export_line_len_];
		out = new char[3*theApp.export_line_len_ + 5];
	}
	catch (std::bad_alloc)
	{
        ::HMessageBox("Insufficient memory");
        theApp.mac_error_ = 10;
		goto func_return;
	}

    const char *hex;
    if (theApp.hex_ucase_)
        hex = "0123456789ABCDEF?";
    else
        hex = "0123456789abcdef?";

    mm->m_wndStatusBar.EnablePaneProgressBar(0, 100);  // turn on progress display
    MSG msg;                                           // use to check for abort key press

    for (curr = start_addr; curr < end_addr; curr += len)
    {
        // Get the data bytes
        len = size_t(min(FILE_ADDRESS(theApp.export_line_len_), end_addr - curr));
        VERIFY(GetDocument()->GetData(buf, len, curr) == len);

        // Convert to hex text
        char *pout = out;

        for(pin = buf; pin < buf+len; ++pin)
        {
            *pout++ = hex[(*pin>>4)&0xF];
            *pout++ = hex[*pin&0xF];
            *pout++ = ' ';
        }
        *pout++ = '\r';
        *pout++ = '\n';

        // Write the string to the file
        try
        {
            ff.Write(out, pout - out);
        }
        catch (CFileException *pfe)
        {
            ::HMessageBox(::FileErrorMessage(pfe, CFile::modeWrite));
            pfe->Delete();
            theApp.mac_error_ = 10;
            goto func_return;
        }

		// Do any redrawing, but nothing else
		while (::PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_NOREMOVE))
		{
			if (::GetMessage(&msg, NULL, WM_PAINT, WM_PAINT))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}

        // Check if a key has been pressed
        if (::PeekMessage(&msg, NULL, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE))
        {
            // Windows does not like to miss key down events (need to match key up events)
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);

            // Remove any characters resulting from keypresses (so they are not inserted into the active file)
            while (::PeekMessage(&msg, NULL, WM_CHAR, WM_CHAR, PM_REMOVE))
                ;

            if (msg.wParam != 'y' &&  // avoid accidental multiple presses of Y key from aborting
                ::HMessageBox("Abort exporting as hex?", MB_YESNO) == IDYES)
            {
				theApp.mac_error_ = 10;
				goto func_return;
            }
        }

		mm->m_wndStatusBar.SetPaneProgress(0, long(((curr - start_addr)*100)/(end_addr - start_addr)));
    }
    ASSERT(curr == end_addr);

func_return:
    ff.Close();
	mm->m_wndStatusBar.EnablePaneProgressBar(0, -1);  // disable progress bar

	if (buf != NULL)
		delete[] buf;
	if (out != NULL)
        delete[] out;

    if (theApp.mac_error_ < 5)
        theApp.SaveToMacro(km_write_file, 10);
}

void CHexEditView::OnUpdateExportHexText(CCmdUI* pCmdUI) 
{
    FILE_ADDRESS start, end;
    GetSelAddr(start, end);
    ASSERT(start >= 0 && start <= end && end <= GetDocument()->length());

    pCmdUI->Enable(start < end);
}

// This is here only so we can put it in context menu for view
void CHexEditView::OnEditFind() 
{
    ((CMainFrame *)AfxGetMainWnd())->OnEditFind();
}

// This is here only so we can put it in context menu for view
void CHexEditView::OnEditReplace() 
{
    ((CMainFrame *)AfxGetMainWnd())->OnEditReplace();
}

// This is here only so we can put it in context menu for view
void CHexEditView::OnEditGoto() 
{
    ((CMainFrame *)AfxGetMainWnd())->OnEditGoto();
}

void CHexEditView::OnCalcSel() 
{
    ((CMainFrame *)AfxGetMainWnd())->OnCalcSel();
}

void CHexEditView::OnUpdateCalcSel(CCmdUI* pCmdUI) 
{
    FILE_ADDRESS start_addr, end_addr;              // Current selection
    GetSelAddr(start_addr, end_addr);
    pCmdUI->Enable(end_addr - start_addr < 9);
}

void CHexEditView::OnEditCut() 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
//    if (!aa->playing_ && GetFocus() != this) SetFocus(); // Ensure focus does not stay in DlgBar

    // Can't delete if view is read only or in overtype mode
    if (check_ro("cut to the clipboard"))
        return;

    if (display_.overtype)
    {
		// xxx direct warning if GetDocument()->IsDevice()?
        if (::HMessageBox("You can't cut while in overtype mode.\r"
                          "Do you want to turn off overtype mode?",
                          MB_OKCANCEL) == IDCANCEL)
        {
            aa->mac_error_ = 5;
            return;
        }
        else if (!do_insert())
			return;
    }

    // Copy the selection to the clipboard and then delete it
    if (!CopyToClipboard())
    {
        ASSERT(aa->mac_error_ > 0);             // There must have been an error
        return;
    }

    // Handle deletion of chars at caret or selection
    FILE_ADDRESS start, end;
    GetSelAddr(start, end);
    ASSERT(start >= 0 && start < end && end <= GetDocument()->length());
    GetDocument()->Change(mod_delforw, start, end-start,
                              NULL, 0, this);
    DisplayCaret();                     // Make sure caret is visible

    CHECK_SECURITY(101);

    aa->SaveToMacro(km_cut);
}

void CHexEditView::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
    if (GetDocument()->read_only())
        pCmdUI->Enable(FALSE);          // disallow cut if file is read only
    else
        OnUpdateClipboard(pCmdUI);
}

void CHexEditView::OnEditCopy()
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
//    if (!aa->playing_ && GetFocus() != this) SetFocus(); // Ensure focus does not stay in DlgBar

    if (CopyToClipboard())
        aa->SaveToMacro(km_copy);
}

// Update handler that turns on certain user interface options (Copy etc) if there
// is a selection -- ie. there is something available to be placed on the clipboard
void CHexEditView::OnUpdateClipboard(CCmdUI* pCmdUI) 
{
    // Is there any text selected?
    CPointAp start, end;
    GetSel(start, end);
    if (pCmdUI->m_pSubMenu != NULL)
    {
        // This happens when popup menu itself is drawn
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION | (start != end ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
    }
    else
        pCmdUI->Enable(start != end);
}

const char *CHexEditView::bin_format_name = "BinaryData";

bool CHexEditView::CopyToClipboard()
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());

    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    // Get the addresses of the selection
    FILE_ADDRESS start, end;
    GetSelAddr(start, end);
    ASSERT(start >= 0 && start <= end && end <= GetDocument()->length());
    if (start == end)
    {
        ASSERT(aa->playing_);
        // Copy to clipboard while nothing selected, presumably in macro playback
        ::HMessageBox("Nothing selected to place on clipboard!");
        aa->mac_error_ = 10;
        return false;
    }

    if (!aa->is_nt_ && end - start > MAX_CLIPBOARD)
    {
        ::HMessageBox("Too much for clipboard!");
        aa->mac_error_ = 2;
        return false;
    }

    if (!OpenClipboard())
    {
        ::HMessageBox("The clipboard is in use!");
        aa->mac_error_ = 10;
        return false;
    }
    if (!::EmptyClipboard())
    {
        ::HMessageBox("Could not delete previous contents of the clipboard!");
        ::CloseClipboard();
        aa->mac_error_ = 10;
        return false;
    }

    CWaitCursor wait;                           // Turn on wait cursor (hourglass)

    // Copy to the clipboard as text
    {
        // Get windows memory to allow data to be put on clipboard
        HANDLE hh;                                      // Windows handle for memory
        unsigned char *p_cb, *pp;                       // Ptrs to + within the clipboard mem
        if ((hh = ::GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, size_t(end-start+1))) == NULL ||
            (p_cb = reinterpret_cast<unsigned char *>(::GlobalLock(hh))) == NULL)
        {
            ::HMessageBox("Insufficient memory for clipboard data");
            ::CloseClipboard();
            aa->mac_error_ = 10;
            return false;
        }

        // Copy the data from the document to the global memory
        unsigned char *buf = new unsigned char[clipboard_buf_len];
        size_t len;
        pp = p_cb;
        for (FILE_ADDRESS curr = start; curr < end; curr += len)
        {
            len = min(clipboard_buf_len, int(end - curr));
            VERIFY(GetDocument()->GetData(buf, len, curr) == len);
            // Copy all characters in buffer to clipboard memory (unless nul)
            unsigned char *end_buf = buf + len;
            if (display_.char_set != CHARSET_EBCDIC)
            {
                for (unsigned char *ss = buf; ss < end_buf; ++ss)
                    if (*ss != '\0')
                        *pp++ = *ss;
            }
            else
            {
                for (unsigned char *ss = buf; ss < end_buf; ++ss)
                    if (e2a_tab[*ss] != '\0')
                        *pp++ = e2a_tab[*ss];
            }
        }
        ASSERT(curr == end);

        // If pp has not been incremented then no valid characters were copied
        if (pp == p_cb)
        {
            aa->mac_error_ = 1;
            ((CMainFrame *)AfxGetMainWnd())->
                StatusBarText("Warning: no valid text bytes - no text placed on clipboard");
        }
        *pp ='\0';

        if (::SetClipboardData(CF_TEXT, hh) == NULL)
        {
            ::GlobalFree(hh);
            ::HMessageBox("Could not place text data on clipboard");
            ::CloseClipboard();
            aa->mac_error_ = 10;
            return false;
        }
        aa->ClipBoardAdd(pp - p_cb);
        // Note: the clipboard now owns the memory so ::GlobalFree(hh) should not be called

        delete[] buf;
        ::GlobalUnlock(hh);
    }


    // Create the "BinaryData" clipboard format (or get it if it already exists)
    UINT bin_format;
    if ((bin_format = ::RegisterClipboardFormat(bin_format_name)) != 0)
    {
        // Get windows memory to allow binary data to be put on clipboard
        HANDLE hh;                                      // Windows handle for memory
        unsigned char *pp;                              // Actual pointer to the memory
        if ((hh = ::GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, size_t(end-start+4))) == NULL ||
            (pp = reinterpret_cast<unsigned char *>(::GlobalLock(hh))) == NULL)
        {
            ::HMessageBox("Insufficient memory: stored to clipboard as text only");
            ::CloseClipboard();
            aa->mac_error_ = 10;
            return false;
        }

        // Add the binary data length to first 4 bytes of BinaryData clipboard memory
        long *pl = reinterpret_cast<long *>(pp);
        *pl = long(end - start);

        // Copy the data from the document to the global memory
        VERIFY(GetDocument()->GetData(pp+4, size_t(end - start), start) == end - start);

        ::GlobalUnlock(hh);

        if (::SetClipboardData(bin_format, hh) == NULL)
        {
            ::GlobalFree(hh);
            ::HMessageBox("Could not place binary data on clipboard: stored as text only");
            ::CloseClipboard();
            aa->mac_error_ = 10;
            return false;
        }
        // Note: the clipboard now owns the memory so ::GlobalFree(hh) should not be called
    }

    if (!::CloseClipboard())
    {
        aa->mac_error_ = 20;
        return false;
    }
    return true;
}


// Copy to clipboard as hex text
void CHexEditView::OnCopyHex() 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());

    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    // Get the addresses of the selection
    FILE_ADDRESS start, end;
    GetSelAddr(start, end);
    ASSERT(start >= 0 && start <= end && end <= GetDocument()->length());
    if (start == end)
    {
        // Copy to clipboard while nothing selected, presumably in macro playback
        ASSERT(aa->playing_);
        ::HMessageBox("Nothing selected to place on clipboard!");
        aa->mac_error_ = 10;
        return;
    }

    // Work out the amount of memory needed (may be slightly more than needed).
    // Allow 3 chars for every byte (2 hex digits + a space), plus
    // 2 chars per line (CR+LF), + 1 trailing nul byte.
    FILE_ADDRESS mem_needed = (end-start)*3+((end-start)/rowsize_+2)*2+1;

    if (!aa->is_nt_ && mem_needed > MAX_CLIPBOARD)
    {
        ::HMessageBox("Too much for clipboard!");
        aa->mac_error_ = 2;
        return;
    }

    if (mem_needed > 500000L)
    {
        CString ss;
        ss.Format("Do you really want to put %sbytes on\n"
                  "the clipboard?  This may take some time.",
                  NumScale(double(mem_needed)));
        if (::HMessageBox(ss, MB_YESNO) != IDYES)
        {
            aa->mac_error_ = 5;
            return;
        }
    }

    if (!OpenClipboard())
    {
        ASSERT(0);
        ::HMessageBox("The clipboard is in use!");
        aa->mac_error_ = 10;
        return;
    }
    if (!::EmptyClipboard())
    {
        ASSERT(0);
        ::HMessageBox("Could not delete previous contents of the clipboard!");
        ::CloseClipboard();
        aa->mac_error_ = 10;
        return;
    }

    CWaitCursor wait;                           // Turn on wait cursor (hourglass)

    {
        // Get windows memory to allow data to be put on clipboard
        HANDLE hh;                              // Windows handle for memory
        char *p_cb, *pp;                        // Ptr to start and within the clipboard text

        if ((hh = ::GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, DWORD(mem_needed))) == NULL ||
            (p_cb = reinterpret_cast<char *>(::GlobalLock(hh))) == NULL)
        {
            ::HMessageBox("Insufficient memory for clipboard data");
            ::CloseClipboard();
            aa->mac_error_ = 10;
            return;
        }

        unsigned char cc;
        const char *hex;
        if (aa->hex_ucase_)
            hex = "0123456789ABCDEF?";
        else
            hex = "0123456789abcdef?";

        pp = p_cb;

        for (FILE_ADDRESS curr = start; curr < end; )
        {
            VERIFY(GetDocument()->GetData(&cc, 1, curr) == 1);
            *pp++ = hex[(cc>>4)&0xF];
            *pp++ = hex[cc&0xF];
            *pp++ = ' ';
            if ((++curr + offset_)%rowsize_ == 0)
            {
                *pp++ = '\r';
                *pp++ = '\n';
            }
        }
        if ((curr + offset_)%rowsize_ != 0)
        {
            // Add line termination at end
            *pp++ = '\r';
            *pp++ = '\n';
        }
        *pp ='\0';

        if (::SetClipboardData(CF_TEXT, hh) == NULL)
        {
            ASSERT(0);
            ::GlobalFree(hh);
            ::HMessageBox("Could not place data on clipboard");
            ::CloseClipboard();
            aa->mac_error_ = 10;
            return;
        }
        aa->ClipBoardAdd(pp - p_cb);
        // Note: the clipboard now owns the memory so ::GlobalFree(hh) should not be called

        ::GlobalUnlock(hh);
    }

    if (!::CloseClipboard())
        aa->mac_error_ = 20;
    else
        aa->SaveToMacro(km_copy_hex);
}

// Copy to cipboard as C source (characters stored as hex ints)
void CHexEditView::OnCopyCchar() 
{
    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    // Get the addresses of the selection
    FILE_ADDRESS start, end;
    GetSelAddr(start, end);
    ASSERT(start >= 0 && start <= end && end <= GetDocument()->length());
    if (start == end)
    {
        // Copy to clipboard while nothing selected, presumably in macro playback
        ASSERT(theApp.playing_);
        ::HMessageBox("Nothing selected to place on clipboard!");
        theApp.mac_error_ = 10;
        return;
    }

    CCopyCSrc dlg;
    if (dlg.DoModal() == IDOK)
    {
        do_copy_src(dlg.type_,
            dlg.type_ == CCopyCSrc::FLOAT ? dlg.float_size_ : dlg.int_size_,
            dlg.int_type_,
            dlg.big_endian_,
            dlg.show_address_,
            dlg.indent_);
    }
}

// do_copy_src: creates text on the clipboard from the current selection based on parameters passed
//   src_type = CCopyCSrc::STRING, CCopyCSrc::CHAR, CCopyCSrc::INT, or CCopyCSrc::FLOAT
//   src_size = 0,1,2,3 for 4 sizes of int, or 0,1 for 2 sizes of float
//   int_type = 0,1,2,3 for how the ints are to be output
//   big_endian = determines the byte order for ints and floats
void CHexEditView::do_copy_src(int src_type, int src_size, int int_type, BOOL big_endian,
                               BOOL show_address, int indent)
{
    FILE_ADDRESS start, end;
    GetSelAddr(start, end);

    //    // Work out the amount of memory needed (may be slightly more than needed).
    //    // Allow 6 chars for every byte ("0x" + 2 hex digits + comma + space), plus
    //    // 7 ("/**/ "+CR+LF) + addr_width_ chars per line, + 1 trailing nul byte.
    //    FILE_ADDRESS mem_needed = (end-start)*6+((end-start)/rowsize_+2)*(7+addr_width_)+1;
    FILE_ADDRESS mem_needed;

    switch (src_type)
    {
    default:
        ASSERT(0);
        // fall through
    case CCopyCSrc::STRING:
        // Max 4 chars per byte + 9 bytes for each line (/**/ ""\r\n) + address for each line + terminator
        mem_needed = (end-start)*4 + ((end-start)/rowsize_+2)*(indent+9+addr_width_) + 2;
        break;
    case CCopyCSrc::CHAR:
    case CCopyCSrc::INT:
    case CCopyCSrc::FLOAT:
        // Max 8 chars per byte + 7 bytes per line (/**/ \r\n) + address per line + terminator
        mem_needed = (end-start)*8 + ((end-start)/rowsize_+2)*(indent+7+addr_width_) + 2;
        break;
    }

    if (!theApp.is_nt_ && mem_needed > MAX_CLIPBOARD)
    {
        ::HMessageBox("Too much for clipboard!");
        theApp.mac_error_ = 2;
        return;
    }

    if (mem_needed > 500000L)
    {
        CString ss;
        ss.Format("Do you really want to put %sbytes on\n"
                  "the clipboard?  This may take some time.",
                  NumScale(double(mem_needed)));
        if (::HMessageBox(ss, MB_YESNO) != IDYES)
        {
            theApp.mac_error_ = 5;
            return;
        }
    }

    if (!OpenClipboard())
    {
        ASSERT(0);
        ::HMessageBox("The clipboard is in use!");
        theApp.mac_error_ = 10;
        return;
    }
    if (!::EmptyClipboard())
    {
        ASSERT(0);
        ::HMessageBox("Could not delete previous contents of the clipboard!");
        ::CloseClipboard();
        theApp.mac_error_ = 10;
        return;
    }

    CWaitCursor wait;                           // Turn on wait cursor (hourglass)

    {
        // Get windows memory to allow data to be put on clipboard
        HANDLE hh;                              // Windows handle for memory
        char *p_cb, *pp;                        // Ptr to start and within the clipboard text
        size_t slen;                            // Number of characters added to ouput buffer by sprintf call

        if ((hh = ::GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, DWORD(mem_needed))) == NULL ||
            (p_cb = reinterpret_cast<char *>(::GlobalLock(hh))) == NULL)
        {
            ::HMessageBox("Insufficient memory for clipboard data");
            ::CloseClipboard();
            theApp.mac_error_ = 10;
            return;
        }
        p_cb[mem_needed-1] = '\xCD';            // Add marker char at end so we can check if buffer was overflowed

        // string containing 128 spaces
        const char *spaces = "                                                                "
                             "                                                                ";
        const char *hex;                        // string containing hex digits (in current selected case)
        if (theApp.hex_ucase_)
            hex = "0123456789ABCDEF?";
        else
            hex = "0123456789abcdef?";

        pp = p_cb;

        // Add any indenting
        ASSERT(indent < 128);
        slen = sprintf(pp, "%.*s", indent, spaces);
        pp += slen;

        if (show_address)
        {
            // Add initial address
            *pp++ = '/';
            *pp++ = '*';
            slen = sprintf(pp, addr_format_, start);
            pp += slen;
            *pp++ = '*';
            *pp++ = '/';
            *pp++ = ' ';
        }

        if (src_type == CCopyCSrc::STRING)
            *pp++ = '"';

        FILE_ADDRESS curr = start;              // Address of current byte being worked on
        FILE_ADDRESS line_end;                  // Address of byte at end of current line
        line_end = ((start + offset_)/rowsize_ + 1)*rowsize_ - offset_;

        // Work out how many bytes in each chunk
        size_t get_len;                         // Size of each chunk (1,2,4, or 8)
        switch (src_type)
        {
        default:
            ASSERT(0);
            // fall through
        case CCopyCSrc::STRING:
        case CCopyCSrc::CHAR:
            get_len = 1;
            break;
        case CCopyCSrc::INT:
            switch (src_size)
            {
            default:
                ASSERT(0);
                // fall through
            case CCopyCSrc::INT_32:
                get_len = 4;
                break;
            case CCopyCSrc::INT_8:
                get_len = 1;
                break;
            case CCopyCSrc::INT_16:
                get_len = 2;
                break;
            case CCopyCSrc::INT_64:
                get_len = 8;
                break;
            }
            break;
        case CCopyCSrc::FLOAT:
            switch (src_size)
            {
            default:
                ASSERT(0);
                // fall through
            case CCopyCSrc::FLOAT_64:
                get_len = 8;
                break;
            case CCopyCSrc::FLOAT_32:
                get_len = 4;
                break;
            }
            break;
        }

        for (curr = start; curr + get_len <= end; )
        {
            unsigned char buf[8];               // Largest is 64 bit int/float (8 bytes)

            VERIFY(GetDocument()->GetData(buf, get_len, curr) == get_len);

            switch (src_type)
            {
            default:
                ASSERT(0);
                // fall through
            case CCopyCSrc::STRING:
//                if (isprint(buf[0]))  // isprint seems to return true for ANSI chars too
                if (buf[0] >= ' ' && buf[0] < 127)
                {
                    // Backslash (\) and double-quote (") must be escaped and also do question
                    // mark (?) to avoid accidentally creating a trigraph sequence (??=, etc)
                    if (strchr("\\\"\?", buf[0]))
                        *pp++ = '\\';
                    *pp++ = buf[0];
                }
                else
                {
                    // Control char or non-ASCII char - display as escape char or in hex
                    const char *check = "\a\b\f\n\r\t\v";       // used in search for escape char
                    const char *display = "abfnrtv0";
                    const char *ps;

                    // Note we output a nul byte as hex just in case it is followed by another
                    // digit.  Since strchr includes the terminating nul byte in the search
                    // we have to explicitly check for it.
                    if (buf[0] != '\0' && (ps = strchr(check, buf[0])) != NULL)
                    {
                        // Ouput C/C++ escape sequence
                        *pp++ = '\\';
                        *pp++ = display[ps-check];
                    }
                    else
                    {
                        // Output using hex escape sequence
                        *pp++ = '\\';
                        *pp++ = 'x';
                        *pp++ = hex[(buf[0]>>4)&0xF];
                        *pp++ = hex[buf[0]&0xF];
                        ASSERT(get_len == 1);

                        // If not at end of line we have to watch that the following char is not a hex digit
                        if (curr + get_len < line_end && curr + get_len < end)
                        {
                            // Not at EOL so get the next character into buf[1]
                            VERIFY(GetDocument()->GetData(buf+1, 1, curr + get_len) == 1);
                            if (isxdigit(buf[1]))
                            {
                                // Terminate the string and start a new one so that the following char
                                // does not become concatenated with the 2 hex digits already output
                                *pp++ = '"';
                                *pp++ = ' ';
                                *pp++ = '"';
                            }
                        }
                    }
                }
                break;
            case CCopyCSrc::CHAR:
                *pp++ = '\'';                                   // put in single quotes
//                if (isprint(buf[0]))
                if (buf[0] >= ' ' && buf[0] < 127)
                {
                    // Backslash (\) and apostrophe or single quote (') must be escaped
                    if (strchr("\\'", buf[0]))
                        *pp++ = '\\';
                    *pp++ = buf[0];
                }
                else
                {
                    // Control char or non-ASCII char - display as escape char or in hex
                    const char *check = "\a\b\f\n\r\t\v\0";
                    const char *display = "abfnrtv0";
                    const char *ps;
                    if ((ps = strchr(check, buf[0])) != NULL)
                    {
                        // Ouput C/C++ escape sequence
                        *pp++ = '\\';
                        *pp++ = display[ps-check];
                    }
                    else
                    {
                        // Output using hex escape sequence
                        *pp++ = '\\';
                        *pp++ = 'x';
                        *pp++ = hex[(buf[0]>>4)&0xF];
                        *pp++ = hex[buf[0]&0xF];
                    }
                }
                *pp++ = '\'';                                   // trailing single quote
                *pp++ = ',';
                *pp++ = ' ';
                break;
            case CCopyCSrc::INT:
                if (big_endian)
                    flip_bytes(buf, get_len);
                switch (src_size)
                {
                default:
                    ASSERT(0);
                    // fall through
                case CCopyCSrc::INT_32:
                    switch (int_type)
                    {
                    default:
                        ASSERT(0);
                        // fall through
                    case CCopyCSrc::INT_UNSIGNED:
                        slen = sprintf(pp, "%10u, ", *(long *)buf);
                        break;
                    case CCopyCSrc::INT_SIGNED:
                        slen = sprintf(pp, "%11d, ", *(long *)buf);
                        break;
                    case CCopyCSrc::INT_OCTAL:
                        slen = sprintf(pp, "%012o, ", *(long *)buf);    // octal with leading zeroes
                        break;
                    case CCopyCSrc::INT_HEX:
                        if (theApp.hex_ucase_)
                            slen = sprintf(pp, "0x%08X, ", *(long *)buf); // 0x then leading zeroes
                        else
                            slen = sprintf(pp, "0x%08x, ", *(long *)buf);
                        break;
                    }
                    break;

                case CCopyCSrc::INT_8:
                    switch (int_type)
                    {
                    default:
                        ASSERT(0);
                        // fall through
                    case CCopyCSrc::INT_UNSIGNED:
                        slen = sprintf(pp, "%3u, ", buf[0]);
                        break;
                    case CCopyCSrc::INT_SIGNED:
                        slen = sprintf(pp, "%4d, ", (signed char)buf[0]);
                        break;
                    case CCopyCSrc::INT_OCTAL:
                        slen = sprintf(pp, "%04o, ", buf[0]);
                        break;
                    case CCopyCSrc::INT_HEX:
                        if (theApp.hex_ucase_)
                            slen = sprintf(pp, "0x%02.2X, ", buf[0]);
                        else
                            slen = sprintf(pp, "0x%02.2x, ", buf[0]);
                        break;
                    }
                    break;
                case CCopyCSrc::INT_16:
                    switch (int_type)
                    {
                    default:
                        ASSERT(0);
                        // fall through
                    case CCopyCSrc::INT_UNSIGNED:
                        slen = sprintf(pp, "%5hu, ", *(short *)buf);
                        break;
                    case CCopyCSrc::INT_SIGNED:
                        slen = sprintf(pp, "%6hd, ", *(short *)buf);
                        break;
                    case CCopyCSrc::INT_OCTAL:
                        slen = sprintf(pp, "%07ho, ", *(short *)buf);
                        break;
                    case CCopyCSrc::INT_HEX:
                        if (theApp.hex_ucase_)
                            slen = sprintf(pp, "0x%04.4hX, ", *(short *)buf);
                        else
                            slen = sprintf(pp, "0x%04.4hx, ", *(short *)buf);
                        break;
                    }
                    break;
                case CCopyCSrc::INT_64:
                    switch (int_type)
                    {
                    default:
                        ASSERT(0);
                        // fall through
                    case CCopyCSrc::INT_UNSIGNED:
                        slen = sprintf(pp, "%20I64u, ", *(__int64 *)buf);
                        break;
                    case CCopyCSrc::INT_SIGNED:
                        slen = sprintf(pp, "%20I64d, ", *(__int64 *)buf);
                        break;
                    case CCopyCSrc::INT_OCTAL:
                        slen = sprintf(pp, "%023I64o, ", *(__int64 *)buf);
                        break;
                    case CCopyCSrc::INT_HEX:
                        if (theApp.hex_ucase_)
                            slen = sprintf(pp, "0x%016I64X, ", *(__int64 *)buf);
                        else
                            slen = sprintf(pp, "0x%016I64x, ", *(__int64 *)buf);
                        break;
                    }
                    break;
                }
                pp += slen;
                ASSERT(*(pp-1) == ' ');  // Checks that slen was correct and trailing space added
                break;
            case CCopyCSrc::FLOAT:
                if (big_endian)
                    flip_bytes(buf, get_len);
                switch (src_size)
                {
                default:
                    ASSERT(0);
                    // fall through
                case CCopyCSrc::FLOAT_64:
                    slen = sprintf(pp, "%22.15g, ", *(double *)buf);
                    break;
                case CCopyCSrc::FLOAT_32:
                    slen = sprintf(pp, "%14.7g, ", *(float *)buf);
                    break;
                }
                pp += slen;
                ASSERT(*(pp-1) == ' ');
                break;
            }

            // Check if we need to start a new line
            if ((curr += get_len) >= line_end || curr >= end)
            {
                // Terminate previous line
                if (src_type == CCopyCSrc::STRING)
                    *pp++ = '"';                // terminate string on this line
                *pp++ = '\r';
                *pp++ = '\n';

                // If this is not the last line
                if (curr < end)
                {
                    // Add any indenting
                    slen = sprintf(pp, "%.*s", indent, spaces);
                    pp += slen;

                    if (show_address)
                    {
                        // Output address (in comments) at the start of the line
                        *pp++ = '/';
                        *pp++ = '*';
                        slen = sprintf(pp, addr_format_, curr);
                        pp += slen;
                        *pp++ = '*';
                        *pp++ = '/';
                        *pp++ = ' ';
                    }

                    if (src_type == CCopyCSrc::STRING)
                        *pp++ = '"';            // start new string on this new line
                }

                // Work out where this next line ends
                line_end += rowsize_;
            }
        }
        *pp ='\0';
        ASSERT(pp-p_cb < mem_needed);
        ASSERT(p_cb[mem_needed-1] == '\xCD');   // Check if buffer was overflowed

        if (::SetClipboardData(CF_TEXT, hh) == NULL)
        {
            ASSERT(0);
            ::GlobalFree(hh);
            ::HMessageBox("Could not place data on clipboard");
            ::CloseClipboard();
            theApp.mac_error_ = 10;
            return;
        }
        theApp.ClipBoardAdd(pp - p_cb);
        // Note: the clipboard now owns the memory so ::GlobalFree(hh) should not be called

        ::GlobalUnlock(hh);
    }

    if (!::CloseClipboard())
        theApp.mac_error_ = 20;
    else
        theApp.SaveToMacro(km_copy_cchar, src_type | 
                                          (src_size<<3) | 
                                          (int_type<<6) | 
                                          (big_endian   ? 0x0200 : 0) |
                                          (show_address ? 0 : 0x0400) |  /* bit off means show */
                                          (__int64(indent&0x7F)<<11));
}

// start+end define the bytes to be replaced
// pp points to the new bytes and len is the number of new bytes
void CHexEditView::do_replace(FILE_ADDRESS start, FILE_ADDRESS end, unsigned char *pp, size_t len)
{
    // Can't replace if view is read only
    if (check_ro("replace"))
        return;

    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    ASSERT(start < end && end <= GetDocument()->length());
    if (display_.overtype && end-start != len)
    {
		// xxx direct warning if GetDocument()->IsDevice()?
        if (::HMessageBox("This replacement requires insert mode..\n"
                          "Do you want to turn on insert mode?", MB_OKCANCEL) != IDOK)
        {
            theApp.mac_error_ = 10;
            return;
        }
        else if (!do_insert())
			return;
    }

    if (display_.overtype || end-start == len)
    {
        GetDocument()->Change(mod_replace, start, len, pp, 0, this);
    }
    else
    {
        GetDocument()->Change(mod_delforw, start, end-start, NULL, 0, this);
        if (len > 0)
            GetDocument()->Change(mod_insert, start, len, pp, 0, this, TRUE);
    }
    int row = 0;
    if (display_.vert_display) row = pos2row(GetCaret());
    SetSel(addr2pos(start+len, row), addr2pos(start+len, row));
    DisplayCaret();
    show_prop();                            // Make sure dialogs don't obscure our changes
    show_calc();
    show_pos();                             // Update tool bar
}

void CHexEditView::OnEditPaste() 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
//    if (!aa->playing_ && GetFocus() != this) SetFocus(); // Ensure focus does not stay in DlgBar

    // Can't paste if view is read only
    if (check_ro("paste"))
        return;

    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    UINT ff = 0;                                // Clipboard format number
    HANDLE hh;                                  // Handle to clipboard memory
    unsigned char *pp;                          // Pointer to actual data

    if (!OpenClipboard())
    {
        ASSERT(0);
        ::HMessageBox("The clipboard is in use!");
        aa->mac_error_ = 10;
        return;
    }

    CWaitCursor wait;                           // Turn on wait cursor (hourglass)

    // Check if there is a "BinaryData" format (added by us or DevStudio)
    while ((ff = EnumClipboardFormats(ff)) != 0)
    {
        CString tt;
        char name[16];
        size_t nlen = ::GetClipboardFormatName(ff, name, 15);
        name[nlen] = '\0';
        if (strcmp(name, bin_format_name) == 0)
        {
            // BINARY DATA
            if ((hh = ::GetClipboardData(ff)) != NULL &&
                (pp = reinterpret_cast<unsigned char *>(::GlobalLock(hh))) != NULL)
            {
                long *pl = reinterpret_cast<long *>(pp);
                FILE_ADDRESS start_addr, end_addr;
                GetSelAddr(start_addr, end_addr);
                int row = 0;
                if (start_addr == end_addr && display_.vert_display)
                    row = pos2row(GetCaret());
                // FILE_ADDRESS addr = GetPos();
                if (display_.overtype && start_addr + *pl > GetDocument()->length())
                {
                    if (::HMessageBox("The paste operation extends past EOF\n"
                                      "which is illegal in overtype mode.\n"
                                      "Do you want to turn on insert mode?", MB_OKCANCEL) != IDOK)
                    {
                        ::CloseClipboard();
                        aa->mac_error_ = 10;
                        return;
                    }
					else if (!do_insert())
						return;
                }
                else if (display_.overtype && end_addr-start_addr != *pl)
                {
                    switch (::HMessageBox("Pasting in overtype mode will overwrite data!\r"
                                      "Do you want to turn on insert mode?",
                                      MB_YESNOCANCEL))
                    {
                    case IDYES:
						if (!do_insert())
							return;
                        break;
                    case IDNO:
                        break; /* do nothing */
                    default:
                        aa->mac_error_ = 5;
                        return;
                    }
                }

                // OK make the change
                if (display_.overtype || end_addr-start_addr == *pl)
                    GetDocument()->Change(mod_replace, start_addr, *pl, pp+4, 0, this);
                else if (start_addr == end_addr)
                    GetDocument()->Change(mod_insert, start_addr, *pl, pp+4, 0, this);
                else
                {
                    GetDocument()->Change(mod_delforw, start_addr, end_addr-start_addr, NULL, 0, this);
                    GetDocument()->Change(mod_insert, start_addr, *pl, pp+4, 0, this, TRUE);
                }
//                SetSel(addr2pos(start_addr), addr2pos(start_addr+*pl));
                SetSel(addr2pos(start_addr+*pl, row), addr2pos(start_addr+*pl, row));
                DisplayCaret();
                ::CloseClipboard();
                show_prop();                            // New char - check props
                show_calc();
                show_pos();                             // Update tool bar
                aa->SaveToMacro(km_paste);
            }
            else
                aa->mac_error_ = 20;    // It's there but couldn't get it
            return;
        }
    }

    // BinaryData format not found so just use text format
    if ((hh = ::GetClipboardData(CF_TEXT)) != NULL &&
        (pp = reinterpret_cast<unsigned char *>(::GlobalLock(hh))) != NULL)
    {
        size_t len = strlen(reinterpret_cast<char *>(pp));
        if (len > 0 && display_.char_set != CHARSET_EBCDIC)        // Bugs in other apps (eg Hedit) might cause len == 0
        {
            // TEXT DATA SAVED AS ASCII
            FILE_ADDRESS start_addr, end_addr;
            GetSelAddr(start_addr, end_addr);
            int row = 0;
            if (start_addr == end_addr && display_.vert_display)
                row = pos2row(GetCaret());
            // FILE_ADDRESS addr = GetPos();
            if (display_.overtype && start_addr + len > GetDocument()->length())
            {
                if (::HMessageBox("The paste operation extends past EOF\n"
                                  "which is illegal in overtype mode.\n"
                                  "Do you want to turn on insert mode?", MB_OKCANCEL) != IDOK)
                {
                    ::CloseClipboard();
                    aa->mac_error_ = 10;
                    return;
                }
				else if (!do_insert())
					return;
            }
            else if (display_.overtype && end_addr-start_addr != len)
            {
                switch (::HMessageBox("Pasting in overtype mode will overwrite data!\r"
                                  "Do you want to turn on insert mode?",
                                  MB_YESNOCANCEL))
                {
                case IDYES:
					if (!do_insert())
						return;
                    break;
                case IDNO:
                    break; /* do nothing */
                default:
                    aa->mac_error_ = 5;
                    return;
                }
            }

            // OK make the change
            if (display_.overtype || end_addr-start_addr == len)
                GetDocument()->Change(mod_replace, start_addr, len, pp, 0, this);
            else if (start_addr == end_addr)
                GetDocument()->Change(mod_insert, start_addr, len, pp, 0, this);
            else
            {
                GetDocument()->Change(mod_delforw, start_addr, end_addr-start_addr, NULL, 0, this);
                GetDocument()->Change(mod_insert, start_addr, len, pp, 0, this, TRUE);
            }
//            SetSel(addr2pos(start_addr), addr2pos(start_addr+len));
            SetSel(addr2pos(start_addr+len, row), addr2pos(start_addr+len, row));
            DisplayCaret();
        }
        else if (len > 0)
        {
            // TEXT DATA SAVED AS EBCDIC
            // Copy from clipboard to temp buffer converting to EBCDIC
            unsigned char *buf = new unsigned char[len];
            size_t newlen = 0;
            for (size_t ii = 0; ii < len; ++ii)
                if (a2e_tab[pp[ii]] != '\0')
                    buf[newlen++] = a2e_tab[pp[ii]];
            if (newlen > 0)
            {
                // Insert the EBCDIC characters
                FILE_ADDRESS start_addr, end_addr;
                GetSelAddr(start_addr, end_addr);
                int row = 0;
                if (start_addr == end_addr && display_.vert_display)
                    row = pos2row(GetCaret());
                // FILE_ADDRESS addr = GetPos();
                if (display_.overtype && start_addr + newlen > GetDocument()->length())
                {
                    if (::HMessageBox("The paste operation extends past EOF\n"
                                      "which is illegal in overtype mode.\n"
                                      "Do you want to turn on insert mode?", MB_OKCANCEL) != IDOK)
                    {
                        ::CloseClipboard();
                        aa->mac_error_ = 10;
                        return;
                    }
					else if (!do_insert())
						return;
                }
                else if (display_.overtype && end_addr-start_addr != newlen)
                {
                    switch (::HMessageBox("Pasting in overtype mode will overwrite data!\r"
                                      "Do you want to turn on insert mode?",
                                      MB_YESNOCANCEL))
                    {
                    case IDYES:
						if (!do_insert())
							return;
                        break;
                    case IDNO:
                        break; /* do nothing */
                    default:
                        aa->mac_error_ = 5;
                        return;
                    }
                }

                // OK make the change
                if (display_.overtype || end_addr-start_addr == newlen)
                    GetDocument()->Change(mod_replace, start_addr, newlen, buf, 0, this);
                else if (start_addr == end_addr)
                    GetDocument()->Change(mod_insert, start_addr, newlen, buf, 0, this);
                else
                {
                    GetDocument()->Change(mod_delforw, start_addr, end_addr-start_addr, NULL, 0, this);
                    GetDocument()->Change(mod_insert, start_addr, newlen, buf, 0, this, TRUE);
                }
//                SetSel(addr2pos(start_addr), addr2pos(start_addr+newlen));
                SetSel(addr2pos(start_addr+newlen, row), addr2pos(start_addr+newlen, row));
                DisplayCaret();
            }
            else
            {
                ::HMessageBox("No valid EBCDIC characters to paste");
                aa->mac_error_ = 2;
            }
            delete[] buf;
        }
        else
        {
            ::HMessageBox("Text on clipboard is not valid ASCII text!");
            aa->mac_error_ = 10;    // Invalid text on clipboard?
        }
    }
    else
    {
        // Paste when nothing to paste, presumably in macro
        ::HMessageBox("There is nothing on the clipboard to paste");
        aa->mac_error_ = 10;
    }

    CHECK_SECURITY(49);

    ::CloseClipboard();
    // This actually records even when there were some errors & probably shouldn't
    show_prop();                            // New char - check props
    show_calc();
    show_pos();                             // Update tool bar
    aa->SaveToMacro(km_paste);
}

void CHexEditView::OnPasteAscii() 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());

    // Can't insert text if view is read only or in overtype mode
    if (check_ro("paste (ASCII)"))
        return;

    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    HANDLE hh;                                  // Handle to clipboard memory
    unsigned char *pp;                          // Pointer to actual data

    if (!OpenClipboard())
    {
        ASSERT(0);
        ::HMessageBox("The clipboard is in use!");
        aa->mac_error_ = 10;
        return;
    }

    CWaitCursor wait;                           // Turn on wait cursor (hourglass)

    if ((hh = ::GetClipboardData(CF_TEXT)) != NULL &&
        (pp = reinterpret_cast<unsigned char *>(::GlobalLock(hh))) != NULL)
    {
        size_t len = strlen(reinterpret_cast<char *>(pp));
        if (len > 0)                    // Bugs in other apps (eg Hedit) might cause len == 0
        {
            FILE_ADDRESS start_addr, end_addr;
            GetSelAddr(start_addr, end_addr);
            int row = 0;
            if (start_addr == end_addr && display_.vert_display)
                row = pos2row(GetCaret());
            // FILE_ADDRESS addr = GetPos();
            if (display_.overtype && start_addr + len > GetDocument()->length())
            {
                if (::HMessageBox("The paste operation extends past EOF\n"
                                  "which is illegal in overtype mode.\n"
                                  "Do you want to turn on insert mode?", MB_OKCANCEL) != IDOK)
                {
                    ::CloseClipboard();
                    aa->mac_error_ = 10;
                    return;
                }
				else if (!do_insert())
					return;
            }
            else if (display_.overtype && end_addr-start_addr != len)
            {
                switch (::HMessageBox("Pasting in overtype mode will overwrite data!\r"
                                  "Do you want to turn on insert mode?",
                                  MB_YESNOCANCEL))
                {
                case IDYES:
					if (!do_insert())
						return;
                    break;
                case IDNO:
                    break; /* do nothing */
                default:
                    aa->mac_error_ = 5;
                    return;
                }
            }

            // OK make the change
            if (display_.overtype || end_addr-start_addr == len)
                GetDocument()->Change(mod_replace, start_addr, len, pp, 0, this);
            else if (start_addr == end_addr)
                GetDocument()->Change(mod_insert, start_addr, len, pp, 0, this);
            else
            {
                GetDocument()->Change(mod_delforw, start_addr, end_addr-start_addr, NULL, 0, this);
                GetDocument()->Change(mod_insert, start_addr, len, pp, 0, this, TRUE);
            }
//            SetSel(addr2pos(start_addr), addr2pos(start_addr+len));
            SetSel(addr2pos(start_addr+len, row), addr2pos(start_addr+len, row));
            DisplayCaret();
        }
        else
            aa->mac_error_ = 20;
    }
    else
    {
        // Paste when nothing to paste, presumably in macro
        ::HMessageBox("There is nothing on the clipboard to paste");
        aa->mac_error_ = 10;
    }

    ::CloseClipboard();
  // This actually records even when there were some errors & probably shouldn't
    aa->SaveToMacro(km_paste_ascii);
}

void CHexEditView::OnPasteEbcdic() 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());

    // Can't insert text if view is read only or in overtype mode
    if (check_ro("paste (EBCDIC)"))
        return;

    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    HANDLE hh;                                  // Handle to clipboard memory
    unsigned char *pp;                          // Pointer to actual data

    if (!OpenClipboard())
    {
        ASSERT(0);
        ::HMessageBox("The clipboard is in use!");
        aa->mac_error_ = 10;
        return;
    }

    CWaitCursor wait;                           // Turn on wait cursor (hourglass)

    if ((hh = ::GetClipboardData(CF_TEXT)) != NULL &&
        (pp = reinterpret_cast<unsigned char *>(::GlobalLock(hh))) != NULL)
    {
        size_t len = strlen(reinterpret_cast<char *>(pp));
        if (len > 0)                    // Bugs in other apps (eg Hedit) might cause len == 0
        {
            // Copy from clipboard to temp buffer converting to EBCDIC
            unsigned char *buf = new unsigned char[len];
            size_t newlen = 0;
            for (size_t ii = 0; ii < len; ++ii)
                if (pp[ii] < 128 && a2e_tab[pp[ii]] != '\0')
                    buf[newlen++] = a2e_tab[pp[ii]];
            if (newlen > 0)
            {
                // Insert the EBCDIC characters
                FILE_ADDRESS start_addr, end_addr;
                GetSelAddr(start_addr, end_addr);
                int row = 0;
                if (start_addr == end_addr && display_.vert_display)
                    row = pos2row(GetCaret());
                // FILE_ADDRESS addr = GetPos();
                if (display_.overtype && start_addr + newlen > GetDocument()->length())
                {
                    if (::HMessageBox("The paste operation extends past EOF\n"
                                      "which is illegal in overtype mode.\n"
                                      "Do you want to turn on insert mode?", MB_OKCANCEL) != IDOK)
                    {
                        ::CloseClipboard();
                        aa->mac_error_ = 10;
                        return;
                    }
					else if (!do_insert())
						return;
                }
                else if (display_.overtype && end_addr-start_addr != newlen)
                {
                    switch (::HMessageBox("Pasting in overtype mode will overwrite data!\r"
                                      "Do you want to turn on insert mode?",
                                      MB_YESNOCANCEL))
                    {
                    case IDYES:
						if (!do_insert())
							return;
                        break;
                    case IDNO:
                        break; /* do nothing */
                    default:
                        aa->mac_error_ = 5;
                        return;
                    }
                }

                // OK make the change
                if (display_.overtype || end_addr-start_addr == newlen)
                    GetDocument()->Change(mod_replace, start_addr, newlen, buf, 0, this);
                else if (start_addr == end_addr)
                    GetDocument()->Change(mod_insert, start_addr, newlen, buf, 0, this);
                else
                {
                    GetDocument()->Change(mod_delforw, start_addr, end_addr-start_addr, NULL, 0, this);
                    GetDocument()->Change(mod_insert, start_addr, newlen, buf, 0, this, TRUE);
                }
//                SetSel(addr2pos(start_addr), addr2pos(start_addr+newlen));
                SetSel(addr2pos(start_addr+newlen, row), addr2pos(start_addr+newlen, row));
                DisplayCaret();
            }
            delete[] buf;
        }
        else
            aa->mac_error_ = 20;
    }
    else
    {
        // Paste when nothing to paste, presumably in macro
        ::HMessageBox("There is nothing on the clipboard to paste");
        aa->mac_error_ = 10;
    }

    ::CloseClipboard();
    aa->SaveToMacro(km_paste_ebcdic);  // Don't record if error (mac_error_ > 3)???
}

// Update handler that turns on user interface options (Paste etc) if there is
// text on the clipboard -- ie. there is text that can be pasted into the document
void CHexEditView::OnUpdateTextPaste(CCmdUI* pCmdUI) 
{
    BOOL bEnable = !GetDocument()->read_only() && ::IsClipboardFormatAvailable(CF_TEXT);

    if (pCmdUI->m_pSubMenu != NULL)
    {
        // This happens when popup menu itself is drawn
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION | (bEnable ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
    }
    else
        pCmdUI->Enable(bEnable);

//    if (GetDocument()->read_only())
//        pCmdUI->Enable(FALSE);                  // Disallow paste if file is read only
//    else
//        pCmdUI->Enable(::IsClipboardFormatAvailable(CF_TEXT));
}

void CHexEditView::OnPasteUnicode() 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());

    // Can't insert text if view is read only or in overtype mode
    if (check_ro("paste (Unicode)"))
        return;

    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    HANDLE hh;                                  // Handle to clipboard memory
    wchar_t *pp;                                // Pointer to actual data

    if (!OpenClipboard())
    {
        ASSERT(0);
        ::HMessageBox("The clipboard is in use!");
        aa->mac_error_ = 10;
        return;
    }

    CWaitCursor wait;                           // Turn on wait cursor (hourglass)

    if ((hh = ::GetClipboardData(CF_UNICODETEXT)) != NULL &&
        (pp = reinterpret_cast<wchar_t *>(::GlobalLock(hh))) != NULL)
    {
        size_t len = wcslen(reinterpret_cast<wchar_t *>(pp));
        if (len > 0)                    // Bugs in other apps (eg Hedit) might cause len == 0
        {
            FILE_ADDRESS start_addr, end_addr;
            GetSelAddr(start_addr, end_addr);
            int row = 0;
            if (start_addr == end_addr && display_.vert_display)
                row = pos2row(GetCaret());
            // FILE_ADDRESS addr = GetPos();
            if (display_.overtype && start_addr + 2*len > GetDocument()->length())
            {
                if (::HMessageBox("The paste operation extends past EOF\n"
                                  "which is illegal in overtype mode.\n"
                                  "Do you want to turn on insert mode?", MB_OKCANCEL) != IDOK)
                {
                    ::CloseClipboard();
                    aa->mac_error_ = 10;
                    return;
                }
				else if (!do_insert())
					return;
            }
            else if (display_.overtype && end_addr-start_addr != 2*len)
            {
                switch (::HMessageBox("Pasting in overtype mode will overwrite data!\r"
                                  "Do you want to turn on insert mode?",
                                  MB_YESNOCANCEL))
                {
                case IDYES:
					if (!do_insert())
						return;
                    break;
                case IDNO:
                    break; /* do nothing */
                default:
                    aa->mac_error_ = 5;
                    return;
                }
            }

            // OK make the change
            if (display_.overtype || end_addr-start_addr == 2*len)
                GetDocument()->Change(mod_replace, start_addr, 2*len, (unsigned char *)pp, 0, this);
            else if (start_addr == end_addr)
                GetDocument()->Change(mod_insert, start_addr, 2*len, (unsigned char *)pp, 0, this);
            else
            {
                GetDocument()->Change(mod_delforw, start_addr, end_addr-start_addr, NULL, 0, this);
                GetDocument()->Change(mod_insert, start_addr, 2*len, (unsigned char *)pp, 0, this, TRUE);
            }
//            SetSel(addr2pos(start_addr), addr2pos(start_addr + 2*len));
            SetSel(addr2pos(start_addr + 2*len, row), addr2pos(start_addr + 2*len, row));
            DisplayCaret();
        }
        else
            aa->mac_error_ = 20;
    }
    else
    {
        // Paste when nothing to paste, presumably in macro
        ::HMessageBox("There is nothing on the clipboard to paste");
        aa->mac_error_ = 10;
    }

    ::CloseClipboard();
      // This actually records even when there were some errors & probably shouldn't
    aa->SaveToMacro(km_paste_unicode);
}

void CHexEditView::OnUpdateUnicodePaste(CCmdUI* pCmdUI) 
{
    BOOL bEnable = !GetDocument()->read_only() && ::IsClipboardFormatAvailable(CF_UNICODETEXT);

    if (pCmdUI->m_pSubMenu != NULL)
    {
        // This happens when popup menu itself is drawn
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION | (bEnable ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
    }
    else
        pCmdUI->Enable(bEnable);

//    if (GetDocument()->read_only())
//        pCmdUI->Enable(FALSE);
//    else
//        pCmdUI->Enable(::IsClipboardFormatAvailable(CF_UNICODETEXT));
}

// Reset all options to the current defaults
void CHexEditView::OnDisplayReset() 
{
    // Change search type in find modeless dlg if open
    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();
    if (theApp.open_display_.char_set != display_.char_set)
	{
		// Character set is to be changed - so update find dlg to match
		if (theApp.open_display_.char_set == CHARSET_EBCDIC)
			mm->m_wndFind.m_pSheet->SetCharSet(CFindSheet::RB_CHARSET_EBCDIC);
		else
			mm->m_wndFind.m_pSheet->SetCharSet(CFindSheet::RB_CHARSET_ASCII);
	}

    begin_change();

    // Change font
    undo_.push_back(view_undo(undo_font));
    if (display_.FontRequired() == FONT_OEM)
    {
        undo_.back().plf = new LOGFONT;
        *(undo_.back().plf) = oem_lf_;
        if (theApp.open_oem_plf_ != NULL)
            oem_lf_ = *theApp.open_oem_plf_;
        else
        {
            memset((void *)&oem_lf_, '\0', sizeof(oem_lf_));
            _tcscpy(oem_lf_.lfFaceName, _T("Terminal")); // The only certain OEM font?
            oem_lf_.lfHeight = 18;
            oem_lf_.lfCharSet = OEM_CHARSET;         // Only allow OEM/IBM character set fonts
        }
    }
    else
    {
        undo_.back().plf = new LOGFONT;
        *(undo_.back().plf) = lf_;
        if (theApp.open_plf_ != NULL)
            lf_ = *theApp.open_plf_;
        else
        {
            memset((void *)&lf_, '\0', sizeof(lf_));
            _tcscpy(lf_.lfFaceName, _T("Courier")); // A nice fixed (no-proportional) font
            lf_.lfHeight = 16;
            lf_.lfCharSet = ANSI_CHARSET;           // Only allow ANSI character set fonts
        }
    }

    // Change autofit
    undo_.push_back(view_undo(undo_autofit, TRUE));
    if (!display_.autofit)
        undo_.back().rowsize = rowsize_;
    else
        undo_.back().rowsize = 0;

    // Note: we don't use disp_state_ = theApp.disp_state_ as this loses edit_char and mark_char settings
    display_.hex_area = theApp.open_display_.hex_area;
    display_.char_area = theApp.open_display_.char_area;
    display_.char_set = theApp.open_display_.char_set;
    display_.control = theApp.open_display_.control;

    display_.autofit = theApp.open_display_.autofit;
    display_.dec_addr = theApp.open_display_.dec_addr;

    display_.readonly = theApp.open_display_.readonly;
    display_.overtype = theApp.open_display_.overtype;
//    display_.keep_times = theApp.open_display_.keep_times;

    display_.vert_display = theApp.open_display_.vert_display;
    display_.borders = theApp.open_display_.borders;
	if (GetDocument()->IsDevice())
	{
		display_.overtype = 1;  // INS not allowed
        display_.borders = 1;   // Always display borders for devices by default
	}

    // Make sure that caret and mark are not in hidden area
    ASSERT(display_.char_area || display_.hex_area);
    if (!display_.hex_area)
    {
        display_.edit_char = TRUE;
        display_.mark_char = TRUE;
    }
    else if (!display_.char_area)
    {
        display_.edit_char = FALSE;
        display_.mark_char = FALSE;
    }
    make_change(TRUE);

    if (rowsize_ != theApp.open_rowsize_)
    {
        undo_.push_back(view_undo(undo_rowsize, TRUE));
        undo_.back().rowsize = rowsize_;            // Save previous rowsize for undo
        rowsize_ = theApp.open_rowsize_;
    }
    if (real_offset_ != theApp.open_offset_)
    {
        undo_.push_back(view_undo(undo_offset, TRUE));
        undo_.back().rowsize = real_offset_;        // Save previous offset for undo
        real_offset_ = offset_ = theApp.open_offset_;
        if (real_offset_ >= rowsize_)
            offset_ = rowsize_ - 1;
    }
    undo_.push_back(view_undo(undo_group_by, TRUE));
    undo_.back().rowsize = group_by_;              // Save previous group_by for undo
    group_by_ = theApp.open_group_by_;

    undo_.push_back(view_undo(undo_scheme, TRUE));      // Allow undo of scheme change
    undo_.back().pscheme_name = new CString;
    *undo_.back().pscheme_name = scheme_name_;

    scheme_name_ = "";          // Force scheme reset
    set_colours();

    end_change();

    theApp.SaveToMacro(km_display_reset, unsigned __int64(0));  // Store zero to allow for future additions
}

void CHexEditView::OnEditUndo() 
{
    CWaitCursor wait;                           // Turn on wait cursor (hourglass)
//    if (!aa->playing_ && GetFocus() != this) SetFocus(); // Ensure focus does not stay in DlgBar

    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    BOOL more = TRUE;

    while (more)
    {
        ASSERT(undo_.size() > 0 || theApp.playing_);
        more = undo_.back().previous_too;
        if (!do_undo())
        {
            theApp.mac_error_ = 10;
            return;
        }
    }

    theApp.SaveToMacro(km_undo);
}

void CHexEditView::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(undo_.size() > 0);
}

void CHexEditView::OnUndoChanges() 
{
    CWaitCursor wait;                           // Turn on wait cursor (hourglass)
    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    BOOL more;
    BOOL last_change = is_last_change();

    do
    {
        ASSERT(undo_.size() > 0 || theApp.playing_);
        more = undo_.back().previous_too;
        if (!do_undo())
        {
            theApp.mac_error_ = 10;
            return;
        }
    } while (undo_.size() > 0 && (more || !(last_change || is_last_change())));
//    } while ((more || (last_change == is_last_change() && undo_.size() > 0));

    theApp.SaveToMacro(km_undo_changes);
}

// Check if the top "operation" on the undo stack is a file change.  Note that an operation
// consists of all elts on the stack back until one without the previous_too flag set.
BOOL CHexEditView::is_last_change()
{
#ifdef _DEBUG
    {
        // Why can't the debugger look into STL containers?
        int undo_size = undo_.size();
        for (std::vector<view_undo>::reverse_iterator rr = undo_.rbegin(); rr != undo_.rend(); ++rr)
        {
            view_undo undo_elt = *rr;
        }
    }
#endif

    // Check if the last thing on undo stack is a change
    for (std::vector<view_undo>::reverse_iterator pp = undo_.rbegin(); pp != undo_.rend(); ++pp)
    {
        if (pp->utype == undo_change)
            return TRUE;         // Top undo op is a change

        if (!pp->previous_too)
            break;               // End of top undo operation
    }
    return FALSE;
}

void CHexEditView::OnUpdateUndoChanges(CCmdUI* pCmdUI) 
{
    BOOL change_present = FALSE;

    // Check if the last thing on undo stack is a change
    for (std::vector<view_undo>::iterator pp = undo_.begin(); pp != undo_.end(); ++pp)
    {
        if (pp->utype == undo_change)
        {
            change_present = TRUE;    // There is a change
            break;
        }
    }

    pCmdUI->Enable(change_present);
}

BOOL CHexEditView::do_undo()
{
    // The string 'mess' is used to display a message in the status
    // bar if it is not obvious what has been undone.
    CString mess, tmp;
    CMainFrame *mm = dynamic_cast<CMainFrame *>(AfxGetMainWnd());

    BOOL caret_displayed = CaretDisplayed();
    FILE_ADDRESS start_addr, end_addr;
    BOOL end_base = GetSelAddr(start_addr, end_addr);
    int row = 0;
    if (start_addr == end_addr && display_.vert_display)
        row = pos2row(GetCaret());

    CHECK_SECURITY(191);
    if (undo_.size() == 0)
    {
        // This can only (presumably) happen during a macro
        ::HMessageBox("There is nothing to undo");
        return FALSE;
    }

    switch (undo_.back().utype)
    {
    case undo_move:
        GoAddress(undo_.back().address);
        show_prop();
        show_calc();
        show_pos();
        DisplayCaret();         // Make sure move visible
        break;
    case undo_sel:
        end_addr = undo_.back().address;

        // Now go back to previous undo (which should be undo_move)
        undo_.pop_back();
        ASSERT(undo_.size() > 0);
        ASSERT(undo_.back().utype == undo_move);
        GoAddress(undo_.back().address, end_addr);
        show_prop();
        show_calc();
        show_pos();
        DisplayCaret();         // Make sure move visible
        break;
    case undo_change:
#if 0
        if (display_.readonly)
        {
            if (::HMessageBox("You can't undo changes while the window is read only.\r"
                              "Do you want to turn off read only mode?",
                              MB_OKCANCEL) == IDCANCEL)
                return FALSE;
            else
                allow_mods();
        }
#endif
        // Note: flag == TRUE if this view originally made the change.
        if (!undo_.back().flag)
            mess += "Undo: changes made in different window undone ";
#ifndef NDEBUG
        if (!GetDocument()->Undo(this, undo_.back().index,
                                 undo_.back().flag))
            return FALSE;
#else
        if (!GetDocument()->Undo(this, 0, undo_.back().flag))
            return FALSE;
#endif
        break;

    case undo_state:
        disp_state_ = undo_.back().disp_state;
        redo_font();
        recalc_display();
        if (end_base)
            SetSel(addr2pos(end_addr, row), addr2pos(start_addr, row), true);
        else
            SetSel(addr2pos(start_addr, row), addr2pos(end_addr, row));
        if (caret_displayed)
            DisplayCaret();                     // Keep caret within display
        DoInvalidate();

        {
            // Make sure calculator big-endian checkbox is correct
            CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();
            if (!theApp.refresh_off_ && mm->m_wndCalc.IsVisible())
                mm->m_wndCalc.update_controls();
        }
        break;

    case undo_overtype:
		ASSERT(!GetDocument()->IsDevice());
        display_.overtype = undo_.back().flag;
        if (GetDocument()->length() == 0)
            DoInvalidate();
        if (display_.overtype)
            mess += "Undo: overtype now ON ";
        else
            mess += "Undo: overtype now OFF ";
        break;

    case undo_readonly:
        display_.readonly = undo_.back().flag;
        if (display_.readonly)
            mess += "Undo: read only now ON ";
        else
            mess += "Undo: read only now OFF ";
        break;

    case undo_font:
        if (display_.FontRequired() == FONT_OEM)
            oem_lf_ = *(undo_.back().plf);
        else
            lf_ = *(undo_.back().plf);

        redo_font();

        // Calculate new position based on new font size
        recalc_display();
        if (end_base)
            SetSel(addr2pos(end_addr, row), addr2pos(start_addr, row), true);
        else
            SetSel(addr2pos(start_addr, row), addr2pos(end_addr, row));
        if (caret_displayed)
            DisplayCaret();                     // Keep caret within display
        DoInvalidate();
        mess += "Undo: font restored ";
        break;

    case undo_scheme:
        tmp = scheme_name_;
        scheme_name_ = *(undo_.back()).pscheme_name;
        if (set_colours())
            DoInvalidate();
        else
        {
            scheme_name_ = tmp;
            if (theApp.is_us_)
                ::HMessageBox("Previous color scheme not found.\n"
                              "The operation could not be undone.");
            else
                ::HMessageBox("Previous colour scheme not found.\n"
                              "The operation could not be undone.");
        }
        break;

    case undo_rowsize:
        rowsize_ = undo_.back().rowsize;
        if (offset_ >= rowsize_)
            offset_ = rowsize_ - 1;
        recalc_display();
        if (end_base)
            SetSel(addr2pos(end_addr, row), addr2pos(start_addr, row), true);
        else
            SetSel(addr2pos(start_addr, row), addr2pos(end_addr, row));
        if (caret_displayed)
            DisplayCaret();     // Keep caret within display
        DoInvalidate();
        break;

    case undo_group_by:
        group_by_ = undo_.back().rowsize;
        recalc_display();
        if (end_base)
            SetSel(addr2pos(end_addr, row), addr2pos(start_addr, row), true);
        else
            SetSel(addr2pos(start_addr, row), addr2pos(end_addr, row));
        if (caret_displayed)
            DisplayCaret();     // Keep caret within display
        DoInvalidate();
        break;

    case undo_offset:
        real_offset_ = offset_ = undo_.back().rowsize;
        if (real_offset_ >= rowsize_)
            offset_ = rowsize_ - 1;
        recalc_display();
        if (end_base)
            SetSel(addr2pos(end_addr, row), addr2pos(start_addr, row), true);
        else
            SetSel(addr2pos(start_addr, row), addr2pos(end_addr, row));
        if (caret_displayed)
            DisplayCaret();     // Keep caret within display
        DoInvalidate();
        break;

    case undo_autofit:
        // If rowsize has been specified then autofit is now off (undo turn on)
        display_.autofit = undo_.back().rowsize == 0;
//      display_.autofit = !display_.autofit;
        if (!display_.autofit)
        {
            mess += "Undo: auto fit now OFF ";
            rowsize_ = undo_.back().rowsize;
            if (offset_ >= rowsize_)
                offset_ = rowsize_ - 1;
        }
        recalc_display();
        if (end_base)
            SetSel(addr2pos(end_addr, row), addr2pos(start_addr, row), true);
        else
            SetSel(addr2pos(start_addr, row), addr2pos(end_addr, row));
        if (caret_displayed)
            DisplayCaret();     // Keep caret within display
        DoInvalidate();
        break;

    case undo_setmark:
        invalidate_addr_range(mark_, mark_ + 1);
        mark_ = undo_.back().address;
		if (theApp.align_mark_ && mark_ != GetDocument()->base_addr_)
		{
			GetDocument()->StopSearch();
			GetDocument()->base_addr_ = mark_;
			GetDocument()->StartSearch();
		}
        invalidate_addr_range(mark_, mark_ + 1);
        show_calc();                    // Status of some buttons may have changed when mark_ moves
        break;

    case undo_highlight:
        hl_set_ = *(undo_.back().phl);
        DoInvalidate();
        break;
    case undo_unknown:
    default:
        ASSERT(0);
        mess += " Unknown undo! ";
    }
    undo_.pop_back();
    mm->StatusBarText(mess);
    return TRUE;
}

void CHexEditView::OnAddrToggle()
{
    begin_change();
    display_.dec_addr = !display_.dec_addr;
    make_change();
    end_change();

    theApp.SaveToMacro(km_addr);
}

void CHexEditView::OnUpdateAddrToggle(CCmdUI* pCmdUI) 
{
    pCmdUI->SetCheck(display_.dec_addr);
}

void CHexEditView::OnGraphicToggle() 
{
    if (!display_.char_area || display_.char_set == CHARSET_EBCDIC)
    {
        CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());

        // Can't toggle graphic chars, presumably in macro playback
        ASSERT(aa->playing_);
        if (!display_.char_area)
            ::HMessageBox("You can't display graphic characters without the char area");
        else
            ::HMessageBox("Graphic characters are not supported for EBCDIC");
        aa->mac_error_ = 2;
        return;
    }

    begin_change();
	if (display_.char_set == CHARSET_ASCII)
		display_.char_set = CHARSET_ANSI;
	else
		display_.char_set = CHARSET_ASCII;
    make_change();
    end_change();

    CHECK_SECURITY(22);
    theApp.SaveToMacro(km_graphic);
}

void CHexEditView::OnUpdateGraphicToggle(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(display_.char_area && display_.char_set != CHARSET_EBCDIC);
    pCmdUI->SetCheck(display_.char_set == CHARSET_ANSI || display_.char_set == CHARSET_OEM);
}

void CHexEditView::OnCharToggle() 
{
    do_chartoggle();
}

void CHexEditView::do_chartoggle(int state /*=-1*/) 
{
    if (display_.vert_display)
    {
        ::HMessageBox("You can't toggle char area in xxx mode");
        theApp.mac_error_ = 2;
        return;
    }

    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());

    begin_change();

    // Change state of char area flag
    if (state == -1)
        display_.char_area = !display_.char_area;
    else if (state != display_.char_area)
        display_.char_area = state;

    // Make sure things are kept consistent
    if (!display_.char_area && display_.edit_char)
    {
        display_.edit_char = FALSE;
        SetHorzBufferZone(2);                      // Allow more room in hex area
    }

    if (!display_.char_area && !display_.hex_area)
    {
        display_.hex_area = TRUE;
    }

    make_change();
    end_change();

    aa->SaveToMacro(km_char_area, state);
}

void CHexEditView::OnUpdateCharToggle(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(!display_.vert_display);
    pCmdUI->SetCheck(display_.char_area);
}

void CHexEditView::do_hextoggle(int state /*=-1*/) 
{
    if (display_.vert_display)
    {
        ::HMessageBox("You can't toggle hex area in xxx mode");
        theApp.mac_error_ = 2;
        return;
    }

    begin_change();
    if (state == -1)
        display_.hex_area = !display_.hex_area;
    else if (state != display_.hex_area)
        display_.hex_area = state;

    // Save previous value of display hex for undo and also swap edit area if nec.
    if (!display_.hex_area && !display_.edit_char)
    {
        display_.edit_char = TRUE;
        SetHorzBufferZone(1);
    }

    if (!display_.hex_area && !display_.char_area)
    {
        display_.char_area = TRUE;
    }

    make_change();
    end_change();

    theApp.SaveToMacro(km_hex_area, state);
}

void CHexEditView::OnOemToggle() 
{
    if (!display_.char_area || display_.char_set == CHARSET_EBCDIC)
    {
        // Can't toggle OEM chars, presumably in macro playback
        ASSERT(theApp.playing_);
        if (!display_.char_area)
            ::HMessageBox("You can't display OEM/ANSI graphic characters without the char area");
        else if (display_.char_set == CHARSET_EBCDIC)
            ::HMessageBox("Graphic characters are not supported for EBCDIC");
        theApp.mac_error_ = 2;
        return;
    }

    begin_change();
	if (display_.char_set == CHARSET_OEM)
		display_.char_set = CHARSET_ANSI;
	else
		display_.char_set = CHARSET_OEM;
    make_change();
    end_change();

    theApp.SaveToMacro(km_oem);
}

void CHexEditView::OnUpdateOemToggle(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(display_.char_area && display_.char_set != CHARSET_EBCDIC);
    pCmdUI->SetCheck(display_.char_set == CHARSET_OEM);
}

void CHexEditView::OnFontInc() 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
//    if (!aa->playing_ && GetFocus() != this) SetFocus(); // Ensure focus does not stay in DlgBar
    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    // Save font for undo
    // xxx this could be done with a size arg rather than a whole LOGFONT structure
    LOGFONT *plf = new LOGFONT;
    *plf = display_.FontRequired() == FONT_OEM ? oem_lf_ : lf_;
    for ( ; ; )
    {
        // Increase font size by one pixel
        CPoint convert(plf->lfWidth, plf->lfHeight);
        CClientDC dc(this);
        OnPrepareDC(&dc);
        dc.LPtoDP(&convert);
        if (convert.y < max_font_size)
            ++convert.y;
        else
        {
            ((CMainFrame *)AfxGetMainWnd())->
                StatusBarText("Warning: Font size too big - not increased");
            aa->mac_error_ = 2;
            return;
        }
        dc.DPtoLP(&convert);
        plf->lfHeight = convert.y;
        plf->lfWidth = 0;            // Calced from height

        CFont font;
        font.CreateFontIndirect(plf);
//        if (display_.char_area && !display_.ebcdic && display_.graphic && display_.oem)
//        {
//            oem_lf_.lfHeight = convert.y;
//            oem_lf_.lfWidth = 0;                        // Calced from height
//            font.CreateFontIndirect(&oem_lf_);
//        }
//        else
//        {
//            lf_.lfHeight = convert.y;
//            lf_.lfWidth = 0;                        // Calced from height
//            font.CreateFontIndirect(&lf_);
//        }
        TEXTMETRIC tm;
        CFont *tf = dc.SelectObject(&font);
        dc.GetTextMetrics(&tm);
        dc.SelectObject(tf);                        // Deselect font before it is destroyed
        if (tm.tmHeight + tm.tmExternalLeading > text_height_)
        {
            if (display_.FontRequired() == FONT_OEM)
                oem_lf_.lfHeight = convert.y;
            else
                lf_.lfHeight = convert.y;
            break;
        }
    }

    BOOL caret_displayed = CaretDisplayed();
//      FILE_ADDRESS address = pos2addr(GetCaret());
    FILE_ADDRESS start_addr, end_addr;
    BOOL end_base = GetSelAddr(start_addr, end_addr);
    int row = 0;
    if (start_addr == end_addr && display_.vert_display)
        row = pos2row(GetCaret());

    // Create and install the new font
    redo_font();

    // Calculate new position (and new total size) based on change in font size
    recalc_display();
    if (end_base)
        SetSel(addr2pos(end_addr, row), addr2pos(start_addr, row), true);
    else
        SetSel(addr2pos(start_addr, row), addr2pos(end_addr, row));
    if (caret_displayed)
        DisplayCaret();                 // Keep caret within display
    DoInvalidate();
    undo_.push_back(view_undo(undo_font));      // Allow undo of font change
    undo_.back().plf = plf;
    aa->SaveToMacro(km_inc_font);
}

void CHexEditView::OnUpdateFontInc(CCmdUI* pCmdUI) 
{
    // Create a large (max_font_size) font and see if the current font is
    // displayed at the same size on screen.  If so we can't increase the size
    LOGFONT logfont;
    logfont = display_.FontRequired() == FONT_OEM ? oem_lf_ : lf_;              // Get font the same as current ...
    {
        CPoint convert(0, max_font_size);       // ... but very big
        CClientDC dc(this);
        OnPrepareDC(&dc);
        dc.DPtoLP(&convert);
        logfont.lfHeight = convert.y;
    }
    logfont.lfWidth = 0;                        // Width calced from height

    // Create font, put into DC, and see what size it would be on screen
    CFont font;
    font.CreateFontIndirect(&logfont);
    {
        TEXTMETRIC tm;
        CClientDC dc(this);
        OnPrepareDC(&dc);
        CFont *tf = dc.SelectObject(&font);
        dc.GetTextMetrics(&tm);
        dc.SelectObject(tf);
        if (tm.tmHeight + tm.tmExternalLeading == text_height_)
            pCmdUI->Enable(FALSE);              // Already at smallest displayable font
    }
}

void CHexEditView::OnFontDec() 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
//    if (!aa->playing_ && GetFocus() != this) SetFocus(); // Ensure focus does not stay in DlgBar
    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    // Save font for undo
    // xxx this could be done with a size arg rather than a whole LOGFONT structure
    LOGFONT *plf = new LOGFONT;
    *plf = display_.FontRequired() == FONT_OEM ? oem_lf_ : lf_;
    for ( ; ; )
    {
        // Decrease font size by one pixel
        CPoint convert(plf->lfWidth, plf->lfHeight);
        CClientDC dc(this);
        OnPrepareDC(&dc);
        dc.LPtoDP(&convert);
        if (convert.y > 1)
            convert.y--;
        else
        {
            ((CMainFrame *)AfxGetMainWnd())->
                StatusBarText("Warning: Font size already at minimum - not decreased");
            aa->mac_error_ = 2;
            return;
        }
        dc.DPtoLP(&convert);
        plf->lfHeight = convert.y;
        plf->lfWidth = 0;            // Calced from height

        CFont font;
        font.CreateFontIndirect(plf);
//        if (display_.char_area && !display_.ebcdic && display_.graphic && display_.oem)
//        {
//            oem_lf_.lfHeight = convert.y;
//            oem_lf_.lfWidth = 0;                        // Calced from height
//            font.CreateFontIndirect(&oem_lf_);
//        }
//        else
//        {
//            lf_.lfHeight = convert.y;
//            lf_.lfWidth = 0;                        // Calced from height
//            font.CreateFontIndirect(&lf_);
//        }
        TEXTMETRIC tm;
        CFont *tf = dc.SelectObject(&font);
        dc.GetTextMetrics(&tm);
        dc.SelectObject(tf);                        // Deselect font before it is destroyed
        if (tm.tmHeight + tm.tmExternalLeading < text_height_)
        {
            if (display_.FontRequired() == FONT_OEM)
                oem_lf_.lfHeight = convert.y;
            else
                lf_.lfHeight = convert.y;
            break;
        }
    }

    BOOL caret_displayed = CaretDisplayed();
//      FILE_ADDRESS address = pos2addr(GetCaret());
    FILE_ADDRESS start_addr, end_addr;
    BOOL end_base = GetSelAddr(start_addr, end_addr);
    int row = 0;
    if (start_addr == end_addr && display_.vert_display)
        row = pos2row(GetCaret());

    // Create and install the new font
    redo_font();

    // Calculate new position (and new total size) based on change in font size
    recalc_display();
    if (end_base)
        SetSel(addr2pos(end_addr, row), addr2pos(start_addr, row), true);
    else
        SetSel(addr2pos(start_addr, row), addr2pos(end_addr, row));
    if (caret_displayed)
        DisplayCaret();                 // Keep caret within display
    DoInvalidate();
    undo_.push_back(view_undo(undo_font));      // Allow undo of font change
    undo_.back().plf = plf;
    aa->SaveToMacro(km_dec_font);
}

void CHexEditView::OnUpdateFontDec(CCmdUI* pCmdUI) 
{
    // If we create a very small font then see what the text height is when that
    // font is used.  If this height is the same as the current text height then the
    // font size can not be decreased any more.
    LOGFONT logfont;
    logfont = display_.FontRequired() == FONT_OEM ? oem_lf_ : lf_;              // Get font the same as current ...
    {
        CPoint convert(0, 1);                   // ... but just one pixel high
        CClientDC dc(this);
        OnPrepareDC(&dc);
        dc.DPtoLP(&convert);
        logfont.lfHeight = convert.y;
    }
    logfont.lfWidth = 0;                        // Width calced from height

    // Create font, put into DC, and see what size it would be on screen
    CFont font;
    font.CreateFontIndirect(&logfont);
    {
        TEXTMETRIC tm;
        CClientDC dc(this);
        OnPrepareDC(&dc);
        CFont *tf = dc.SelectObject(&font);
        dc.GetTextMetrics(&tm);
        dc.SelectObject(tf);
        if (tm.tmHeight + tm.tmExternalLeading == text_height_)
            pCmdUI->Enable(FALSE);              // Already at smallest displayable font
    }
}

void CHexEditView::OnFont() 
{
    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    LOGFONT logfont = display_.FontRequired() == FONT_OEM ? oem_lf_ : lf_;

    // Convert font size to units that user can relate to
    {
        CPoint convert(logfont.lfWidth, logfont.lfHeight);
        CClientDC dc(this);
        OnPrepareDC(&dc);
        dc.LPtoDP(&convert);
        logfont.lfWidth = convert.x;
        logfont.lfHeight = convert.y;
    }

    CFontDialog dlg;
    dlg.m_cf.lpLogFont = &logfont;
    dlg.m_cf.Flags |= CF_INITTOLOGFONTSTRUCT | CF_SHOWHELP;
    dlg.m_cf.Flags &= ~(CF_EFFECTS);              // Disable selection of strikethrough, colours etc
    if (dlg.DoModal() == IDOK)
    {
        // Convert font size back to logical units
        dlg.GetCurrentFont(&logfont);
        if (logfont.lfHeight < 0)
            logfont.lfHeight = -logfont.lfHeight;

        {
            CPoint convert(logfont.lfWidth, logfont.lfHeight);
            CClientDC dc(this);
            OnPrepareDC(&dc);
            dc.DPtoLP(&convert);
            logfont.lfWidth = convert.x;
            logfont.lfHeight = convert.y;
        }

        do_font(&logfont);
    }
    else
    {
        ((CHexEditApp *)AfxGetApp())->mac_error_ = 2;
    }

    // BG search finished message may be lost if modeless dlg running
    ((CHexEditApp *)AfxGetApp())->CheckBGSearchFinished();
}

void CHexEditView::OnFontName() 
{
    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    CHexEditFontCombo* pSrcCombo = 
        (CHexEditFontCombo*)CBCGToolbarComboBoxButton::GetByCmd(IDC_FONTNAME, TRUE);
    if (pSrcCombo == NULL)
    {
        OnFont();
        return;
    }

    LOGFONT logfont = display_.FontRequired() == FONT_OEM ? oem_lf_ : lf_;

    CString str = pSrcCombo->GetText();
    if (pSrcCombo->SetFont(str))
    {
        const CBCGFontDesc *pDesc = pSrcCombo->GetFontDesc();
        ASSERT_VALID(pDesc);
        ASSERT(pDesc->m_strName.GetLength() < LF_FACESIZE);

        strncpy(logfont.lfFaceName, pDesc->m_strName, LF_FACESIZE);
        logfont.lfCharSet = pDesc->m_nCharSet;
        logfont.lfPitchAndFamily = pDesc->m_nPitchAndFamily;

        do_font(&logfont);
    }

    // BG search finished message may be lost if modeless dlg running
    ((CHexEditApp *)AfxGetApp())->CheckBGSearchFinished();
}

void CHexEditView::OnUpdateFontName(CCmdUI* pCmdUI)
{
    if (pCmdUI->m_pOther != NULL && pCmdUI->m_pOther->GetDlgCtrlID() == IDC_FONTNAME)
    {
        // Get parent of control so that we can find it in the list of IDC_FONTNAME buttons
        CWnd *pparent = pCmdUI->m_pOther->GetParent();

        // Get list of IDC_FONTNAME buttons
        CObList listButtons;
        if (CBCGToolBar::GetCommandButtons(IDC_FONTNAME, listButtons) > 0)
        {
            // Search the list until we find the control we are updating
            for (POSITION posCombo = listButtons.GetHeadPosition(); posCombo != NULL; )
            {
                CHexEditFontCombo* pCombo = 
                    DYNAMIC_DOWNCAST(CHexEditFontCombo, listButtons.GetNext(posCombo));

                // Get the real (Windows) combo box from the BCG control
                ASSERT(pCombo != NULL);
                CComboBox *pcb = pCombo->GetComboBox();
                HWND focusWnd = ::GetFocus();

                if (pparent == pcb->GetParent() &&
                    focusWnd != (HWND)0 &&
                    !::IsChild(pCombo->GetHwnd(), focusWnd) && 
                    !pcb->GetDroppedState() )
                {
                    // If the control was found and it was not in the dropped state then update the list
                    if (display_.FontRequired() == FONT_OEM)
                    {
                        pCombo->FixFontList(OEM_CHARSET);
                        pCombo->SetText(oem_lf_.lfFaceName);
                    }
                    else
                    {
                        pCombo->FixFontList(ANSI_CHARSET);
                        pCombo->SetText(lf_.lfFaceName);
                    }
                    break;
                }
            }
        }
    }
}

void CHexEditView::OnFontSize()
{
    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    CHexEditFontSizeCombo* pSrcCombo = 
        (CHexEditFontSizeCombo*)CBCGToolbarComboBoxButton::GetByCmd(IDC_FONTSIZE, TRUE);
    if (pSrcCombo == NULL)
    {
        OnFont();
        return;
    }

    LOGFONT logfont = display_.FontRequired() == FONT_OEM ? oem_lf_ : lf_;

    int nSize = pSrcCombo->GetTwipSize();
    if (nSize == -2 || (nSize >= 0 && nSize < 20) || nSize > 32760)
    {
        AfxMessageBox("Invalid font size.");
    }
    else if (nSize > 0)
    {
        CClientDC dc(this);
        OnPrepareDC(&dc);
        logfont.lfHeight = int((nSize * dc.GetDeviceCaps(LOGPIXELSX)) / 1440.0 + 0.5);

        do_font(&logfont);

        // Store the exact font size (due to rounding probs do_font (SetFont) calc may be slightly off)
        fontsize_.Format("%d", nSize/20);
    }
    // BG search finished message may be lost if modeless dlg running
    ((CHexEditApp *)AfxGetApp())->CheckBGSearchFinished();
}

void CHexEditView::OnUpdateFontSize(CCmdUI* pCmdUI)
{
    if (pCmdUI->m_pOther != NULL && pCmdUI->m_pOther->GetDlgCtrlID() == IDC_FONTSIZE)
    {
        // Get parent of control so that we can find it in the list of IDC_FONTSIZE buttons
        CWnd *pparent = pCmdUI->m_pOther->GetParent();

        // Get list of IDC_FONTSIZE buttons
        CObList listButtons;
        if (CBCGToolBar::GetCommandButtons(IDC_FONTSIZE, listButtons) > 0)
        {
            POSITION posCombo;

            // Don't update any if in edit control or has list dropped for any 
            // of the IDC_FONTSIZE buttons, otherwise they all stuff up
            for (posCombo = listButtons.GetHeadPosition(); posCombo != NULL; )
            {
                CBCGToolbarFontSizeCombo* pCombo = 
                    DYNAMIC_DOWNCAST(CBCGToolbarFontSizeCombo, listButtons.GetNext(posCombo));

                // Get the real (Windows) combo box from the BCG control
                ASSERT(pCombo != NULL);
                CComboBox *pcb = pCombo->GetComboBox();

                if (pcb->GetDroppedState() || ::GetFocus() == ::GetWindow(pcb->m_hWnd, GW_CHILD))
                {
                    return;
                }
            }
            // Search the list until we find the control we are updating
            for (posCombo = listButtons.GetHeadPosition(); posCombo != NULL; )
            {
                CHexEditFontSizeCombo* pCombo = 
                    DYNAMIC_DOWNCAST(CHexEditFontSizeCombo, listButtons.GetNext(posCombo));

                // Get the real (Windows) combo box from the BCG control
                if (pCombo == NULL)
                    continue;
                CComboBox *pcb = pCombo->GetComboBox();
                HWND focusWnd = ::GetFocus();

                if (pparent == pcb->GetParent() &&
                    focusWnd != (HWND)0 &&
                    !::IsChild(pCombo->GetHwnd(), focusWnd) && 
                    !pcb->GetDroppedState() )
                {
                    pCombo->SetText(fontsize_);

                    // Update the list of sizes
                    CHexEditFontCombo *pFontCombo = 
                        (CHexEditFontCombo*)CBCGToolbarComboBoxButton::GetByCmd(IDC_FONTNAME);
                    const CBCGFontDesc *pDesc;
                    if (pFontCombo != NULL && 
                        pFontCombo->OKToRead() &&
                        int(pDesc = pFontCombo->GetFontDesc()) != -1)
                    {
                        int nSize = atoi(fontsize_)*20;
                        if (nSize == -2 || (nSize >= 0 && nSize < 20) || nSize > 32760)
                        {
                            nSize = pCombo->GetTwipsLast ();
                        }

                        if (pDesc->m_strName != pCombo->font_name_)
                        {
                            pCombo->RebuildFontSizes(pDesc->m_strName);
                            pCombo->font_name_ = pDesc->m_strName;
                            TRACE1("$$$$$ rebuilt font sizes %s\n", pCombo->font_name_);
                        }
                        pcb->SetCurSel(-1);
                        pCombo->SetTwipSize(nSize);

                        // Store the exact font size (due to rounding probs do_font (SetFont) calc may be slightly off)
                        fontsize_.Format("%d", nSize/20);
                    }
                    break;
                }
            }
        }
    }
}

void CHexEditView::do_font(LOGFONT *plf)
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();

    // Save current LOGFONT for undo
    LOGFONT *prev_lf = new LOGFONT;
    if (display_.FontRequired() == FONT_OEM)
    {
        // We can't switch to an ANSI char set because we are displaying OEM graphics
        if (plf->lfCharSet == ANSI_CHARSET)
        {
            mm->StatusBarText("Can't switch to ANSI font when displaying IBM/OEM graphics chars");
            aa->mac_error_ = 2;
            return;
        }
        *prev_lf = oem_lf_;
        oem_lf_ = *plf;
    }
    else
    {
        // We can't switch to an OEM char set
        if (plf->lfCharSet == OEM_CHARSET)
        {
            mm->StatusBarText("Can't switch to this font unless displaying IBM/OEM graphics chars");
            aa->mac_error_ = 2;
            return;
        }
        *prev_lf = lf_;
        lf_ = *plf;
    }

    // Set new LOGFONT

    BOOL caret_displayed = CaretDisplayed();
    FILE_ADDRESS start_addr, end_addr;
    BOOL end_base = GetSelAddr(start_addr, end_addr);
    int row = 0;
    if (start_addr == end_addr && display_.vert_display)
        row = pos2row(GetCaret());

    CHECK_SECURITY(9);

    // Create and install the new font
    redo_font();

    // Calculate new position (and new total size) based on change in font size
    recalc_display();
    if (end_base)
        SetSel(addr2pos(end_addr, row), addr2pos(start_addr, row), true);
    else
        SetSel(addr2pos(start_addr, row), addr2pos(end_addr, row));
    if (caret_displayed)
        DisplayCaret();                 // Keep caret within display
    DoInvalidate();
    undo_.push_back(view_undo(undo_font));      // Allow undo of font change
    undo_.back().plf = prev_lf;

    ((CHexEditApp *)AfxGetApp())->SaveToMacro(km_font, 
                                display_.FontRequired() == FONT_OEM ? &oem_lf_ : &lf_);
}

void CHexEditView::change_rowsize(int rowsize)
{
    if (rowsize != rowsize_)
    {
        FILE_ADDRESS start_addr, end_addr;
        BOOL end_base = GetSelAddr(start_addr, end_addr);
        int row = 0;
        if (start_addr == end_addr && display_.vert_display)
            row = pos2row(GetCaret());

        undo_.push_back(view_undo(undo_rowsize));
        undo_.back().rowsize = rowsize_;            // Save previous rowsize for undo
        rowsize_ = rowsize;
        recalc_display();

        if (end_base)
            SetSel(addr2pos(end_addr, row), addr2pos(start_addr, row), true);
        else
            SetSel(addr2pos(start_addr, row), addr2pos(end_addr, row));
        DoInvalidate();
    }
    ((CHexEditApp *)AfxGetApp())->SaveToMacro(km_rowsize, rowsize);
}

void CHexEditView::change_group_by(int group_by)
{
    if (group_by != group_by_)
    {
        FILE_ADDRESS start_addr, end_addr;
        BOOL end_base = GetSelAddr(start_addr, end_addr);
        int row = 0;
        if (start_addr == end_addr && display_.vert_display)
            row = pos2row(GetCaret());

        undo_.push_back(view_undo(undo_group_by));
        undo_.back().rowsize = group_by_;           // Save previous group_by for undo
        group_by_ = group_by;
        recalc_display();

        if (end_base)
            SetSel(addr2pos(end_addr, row), addr2pos(start_addr, row), true);
        else
            SetSel(addr2pos(start_addr, row), addr2pos(end_addr, row));
        DoInvalidate();
    }
    ((CHexEditApp *)AfxGetApp())->SaveToMacro(km_group_by, group_by);
}

void CHexEditView::change_offset(int offset)
{
    if (offset != real_offset_)
    {
        FILE_ADDRESS start_addr, end_addr;
        BOOL end_base = GetSelAddr(start_addr, end_addr);
        int row = 0;
        if (start_addr == end_addr && display_.vert_display)
            row = pos2row(GetCaret());

        undo_.push_back(view_undo(undo_offset));
        undo_.back().rowsize = real_offset_;        // Save previous offset for undo
        real_offset_ = offset_ = offset;
        recalc_display();

        if (end_base)
            SetSel(addr2pos(end_addr, row), addr2pos(start_addr, row), true);
        else
            SetSel(addr2pos(start_addr, row), addr2pos(end_addr, row));
        DoInvalidate();
    }
    ((CHexEditApp *)AfxGetApp())->SaveToMacro(km_offset, offset);
}

void CHexEditView::OnAutoFit() 
{
    do_autofit();
}

// Change autofit mode to state (0 or 1).  If state is -1 then toggle autofit.
void CHexEditView::do_autofit(int state /*=-1*/) 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();

//    if (!aa->playing_ && GetFocus() != this) SetFocus(); // Ensure focus does not stay in DlgBar
    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    BOOL caret_displayed = CaretDisplayed();
//    FILE_ADDRESS address = pos2addr(GetCaret());
    FILE_ADDRESS start_addr, end_addr;
    BOOL end_base = GetSelAddr(start_addr, end_addr);
    int row = 0;
    if (start_addr == end_addr && display_.vert_display)
        row = pos2row(GetCaret());

    if (state == -1)
        display_.autofit = !display_.autofit;
    else if ((BOOL)state == display_.autofit)
        return;                                 // No change - do nothing
    else
        display_.autofit = (BOOL)state;
    undo_.push_back(view_undo(undo_autofit));
    if (display_.autofit)
        undo_.back().rowsize = rowsize_;
    else
        undo_.back().rowsize = 0;
    if (!display_.autofit && real_offset_ != offset_)
    {
        // If autofit turned off but offset has been squeezed then save so it's undone
        undo_.push_back(view_undo(undo_offset, TRUE));
        undo_.back().rowsize = real_offset_;    // Save previous offset for undo
    }
    recalc_display();
//    SetCaret(addr2pos(address));
    if (end_base)
        SetSel(addr2pos(end_addr, row), addr2pos(start_addr, row), true);
    else
        SetSel(addr2pos(start_addr, row), addr2pos(end_addr, row));
    if (caret_displayed)
        DisplayCaret(); // Keep caret within display
    DoInvalidate();

    aa->SaveToMacro(km_autofit, state);

    CHECK_SECURITY(19);
}

void CHexEditView::OnUpdateAutofit(CCmdUI* pCmdUI) 
{
    pCmdUI->SetCheck(display_.autofit);
}

void CHexEditView::OnColumnDec() 
{
    if (rowsize_ > 4)
    {
        FILE_ADDRESS start_addr, end_addr;
        BOOL end_base = GetSelAddr(start_addr, end_addr);
        int row = 0;
        if (start_addr == end_addr && display_.vert_display)
            row = pos2row(GetCaret());

        undo_.push_back(view_undo(undo_rowsize));
        undo_.back().rowsize = rowsize_;            // Save previous rowsize for undo
        rowsize_ = rowsize_ - 1;
        recalc_display();

        if (end_base)
            SetSel(addr2pos(end_addr, row), addr2pos(start_addr, row), true);
        else
            SetSel(addr2pos(start_addr, row), addr2pos(end_addr, row));
        DoInvalidate();
    }
    ((CHexEditApp *)AfxGetApp())->SaveToMacro(km_rowsize, -1);
}

void CHexEditView::OnUpdateColumnDec(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(!display_.autofit && rowsize_ > 4);
}

void CHexEditView::OnColumnInc() 
{
    if (rowsize_ < max_buf)
    {
        FILE_ADDRESS start_addr, end_addr;
        BOOL end_base = GetSelAddr(start_addr, end_addr);
        int row = 0;
        if (start_addr == end_addr && display_.vert_display)
            row = pos2row(GetCaret());

        undo_.push_back(view_undo(undo_rowsize));
        undo_.back().rowsize = rowsize_;            // Save previous rowsize for undo
        rowsize_ = rowsize_ + 1;
        recalc_display();

        if (end_base)
            SetSel(addr2pos(end_addr, row), addr2pos(start_addr, row), true);
        else
            SetSel(addr2pos(start_addr, row), addr2pos(end_addr, row));
        DoInvalidate();
    }
    ((CHexEditApp *)AfxGetApp())->SaveToMacro(km_rowsize, -2);
}

void CHexEditView::OnUpdateColumnInc(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(!display_.autofit && rowsize_ < max_buf);
}

void CHexEditView::redo_font()
{
    CFont *tf = pfont_;
    pfont_ = new CFont;
    pfont_->CreateFontIndirect(display_.FontRequired() == FONT_OEM ? &oem_lf_ : &lf_);
    SetFont(pfont_);
    if (tf != NULL)
        delete tf;                      // Delete old font after it's deselected
}

void CHexEditView::begin_change()
{
    ASSERT(previous_state_ == 0);
    previous_caret_displayed_ = CaretDisplayed();
    previous_end_base_ = GetSelAddr(previous_start_addr_, previous_end_addr_);
    previous_row_ = 0;
    if (previous_start_addr_ == previous_end_addr_ && display_.vert_display)
        previous_row_ = pos2row(GetCaret());
    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    CHECK_SECURITY(21);

    previous_state_ = disp_state_;
}

// Return ptoo or TRUE if it added to the undo stack
BOOL CHexEditView::make_change(BOOL ptoo /*=FALSE*/)
{
    if (previous_state_ != disp_state_)
    {
        if (theApp.intelligent_undo_ &&         // intelligent undo is turned on
            undo_.size() > 0 &&                 // there is an undo op on the stack
            undo_.back().utype == undo_state && // previous op was a state change
            !undo_.back().previous_too &&       // and not part of another operation
            undo_.back().disp_state == disp_state_) // and same state
        {
            ASSERT(!ptoo);     // if part of larger op then previous utype should not have been undo_state
            undo_.pop_back();
        }
        else
        {
            undo_.push_back(view_undo(undo_state, ptoo));
            undo_.back().disp_state = previous_state_;
            ptoo = TRUE;
        }
    }

    return ptoo;
}

void CHexEditView::end_change()
{
    redo_font();
    recalc_display();
    if (!display_.vert_display) previous_row_ = 0;  // If vert mode turned off make sure row is zero
    if (previous_end_base_)
        SetSel(addr2pos(previous_end_addr_, previous_row_), addr2pos(previous_start_addr_, previous_row_), true);
    else
        SetSel(addr2pos(previous_start_addr_, previous_row_), addr2pos(previous_end_addr_, previous_row_));
    if (previous_caret_displayed_)
        DisplayCaret();                 // Keep caret within display
    DoInvalidate();

    // Make sure calculator big-endian checkbox is correct
    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();
    if (!theApp.refresh_off_ && mm->m_wndCalc.IsVisible())
        mm->m_wndCalc.update_controls();

    previous_state_ = 0;  // Used to make sure begin_change/end_change are in pairs
}

void CHexEditView::OnDisplayHex()
{
    // Change the current state (storing previous state in undo vector if changed)
    begin_change();
    display_.hex_area = TRUE;
    display_.char_area = FALSE;
    display_.edit_char = FALSE;
    display_.vert_display = FALSE;
    SetHorzBufferZone(2);
    make_change();
    end_change();

    theApp.SaveToMacro(km_area, 1);
}

void CHexEditView::OnUpdateDisplayHex(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(!display_.vert_display && display_.hex_area && !display_.char_area);
}

void CHexEditView::OnDisplayChar()
{
    // Change the current state (storing previous state in undo vector if changed)
    begin_change();
    display_.hex_area = FALSE;
    display_.char_area = TRUE;
    display_.edit_char = TRUE;
    display_.vert_display = FALSE;
    SetHorzBufferZone(1);
    make_change();
    end_change();

    theApp.SaveToMacro(km_area, 2);
}

void CHexEditView::OnUpdateDisplayChar(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(!display_.vert_display && !display_.hex_area && display_.char_area);
}

void CHexEditView::OnDisplayBoth()
{
    // Change the current state (storing previous state in undo vector if changed)
    begin_change();
    CHECK_SECURITY(50);
    display_.hex_area = TRUE;
    display_.char_area = TRUE;
    display_.vert_display = FALSE;
    if (display_.edit_char)
        SetHorzBufferZone(1);
    else
        SetHorzBufferZone(2);
    make_change();
    end_change();

    theApp.SaveToMacro(km_area, 3);
}

void CHexEditView::OnUpdateDisplayBoth(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(!display_.vert_display && display_.hex_area && display_.char_area);
}

void CHexEditView::OnDisplayStacked()
{
    // Change the current state (storing previous state in undo vector if changed)
    begin_change();
    display_.vert_display = TRUE;
    if (GetVertBufferZone() < 2)
        SetVertBufferZone(2);   // Make sure we can always see the other 2 rows at the same address
    SetHorzBufferZone(1);
    make_change();
    end_change();
    CHECK_SECURITY(30);

    theApp.SaveToMacro(km_area, 4);
}

void CHexEditView::OnUpdateDisplayStacked(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(display_.vert_display);
}

void CHexEditView::OnCharsetAscii()
{
    bool std_scheme = false;

    if (!display_.char_area)
    {
        ASSERT(theApp.playing_);
        ::HMessageBox("You can't change characters sets without the char area");
        theApp.mac_error_ = 2;
        return;
    }

    // Change search type in find modeless dlg
    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();
    if (display_.char_set == CHARSET_EBCDIC)
        mm->m_wndFind.m_pSheet->SetCharSet(CFindSheet::RB_CHARSET_ASCII);

    if (scheme_name_ == ANSI_NAME   && display_.char_set == CHARSET_ANSI ||
        scheme_name_ == OEM_NAME    && display_.char_set == CHARSET_OEM  ||
        scheme_name_ == EBCDIC_NAME && display_.char_set == CHARSET_EBCDIC )
    {
        std_scheme = true;
    }

    begin_change();
	display_.char_set = CHARSET_ASCII;
    BOOL ptoo = make_change();
    if (std_scheme)
    {
        undo_.push_back(view_undo(undo_scheme, ptoo));      // Allow undo of scheme change
        undo_.back().pscheme_name = new CString;
        *undo_.back().pscheme_name = scheme_name_;
        scheme_name_ = ASCII_NAME;
        set_colours();
    }
    end_change();

    CHECK_SECURITY(51);
    theApp.SaveToMacro(km_charset, unsigned __int64(0));
}

void CHexEditView::OnUpdateCharsetAscii(CCmdUI *pCmdUI)
{
    if (pCmdUI->m_pSubMenu != NULL)
    {
        // This happens when popup menu itself is drawn
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION | (display_.char_area ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
    }
    else
    {
        pCmdUI->Enable(display_.char_area);
        pCmdUI->SetCheck(display_.char_set == CHARSET_ASCII);
    }
}

void CHexEditView::OnCharsetAnsi()
{
    bool std_scheme = false;

    if (!display_.char_area)
    {
        ASSERT(theApp.playing_);
        ::HMessageBox("You can't change characters sets without the char area");
        theApp.mac_error_ = 2;
        return;
    }

    // Change search type in find modeless dlg
    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();
    ASSERT(mm != NULL);
    if (display_.char_set == CHARSET_EBCDIC)
        mm->m_wndFind.m_pSheet->SetCharSet(CFindSheet::RB_CHARSET_ASCII);

    if (scheme_name_ == ASCII_NAME  && display_.char_set == CHARSET_ASCII ||
        scheme_name_ == OEM_NAME    && display_.char_set == CHARSET_OEM   ||
        scheme_name_ == EBCDIC_NAME && display_.char_set == CHARSET_EBCDIC)
    {
        std_scheme = true;
    }

    begin_change();
	display_.char_set = CHARSET_ANSI;
    BOOL ptoo = make_change();
    if (std_scheme)
    {
        undo_.push_back(view_undo(undo_scheme, ptoo));      // Allow undo of scheme change
        undo_.back().pscheme_name = new CString;
        *undo_.back().pscheme_name = scheme_name_;
        scheme_name_ = ANSI_NAME;
        set_colours();
    }
    end_change();

    theApp.SaveToMacro(km_charset, 1);
}

void CHexEditView::OnUpdateCharsetAnsi(CCmdUI *pCmdUI)
{
    if (pCmdUI->m_pSubMenu != NULL)
    {
        // This happens when popup menu itself is drawn
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION | (display_.char_area ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
    }
    else
    {
        pCmdUI->Enable(display_.char_area);
        pCmdUI->SetCheck(display_.char_set == CHARSET_ANSI);
    }
}

void CHexEditView::OnCharsetOem()
{
    bool std_scheme = false;

    if (!display_.char_area)
    {
        ASSERT(theApp.playing_);
        ::HMessageBox("You can't change characters sets without the char area");
        theApp.mac_error_ = 2;
        return;
    }

    // Change search type in find modeless dlg
    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();
    ASSERT(mm != NULL);
    if (display_.char_set == CHARSET_EBCDIC)
        mm->m_wndFind.m_pSheet->SetCharSet(CFindSheet::RB_CHARSET_ASCII);

    if (scheme_name_ == ASCII_NAME  && display_.char_set == CHARSET_ASCII ||
        scheme_name_ == ANSI_NAME   && display_.char_set == CHARSET_ANSI  ||
        scheme_name_ == EBCDIC_NAME && display_.char_set == CHARSET_EBCDIC)
    {
        std_scheme = true;
    }

    begin_change();
	display_.char_set = CHARSET_OEM;
    BOOL ptoo = make_change();
    if (std_scheme)
    {
        undo_.push_back(view_undo(undo_scheme, ptoo));      // Allow undo of scheme change
        undo_.back().pscheme_name = new CString;
        *undo_.back().pscheme_name = scheme_name_;
        scheme_name_ = OEM_NAME;
        set_colours();
    }
    end_change();

    theApp.SaveToMacro(km_charset, 2);
}

void CHexEditView::OnUpdateCharsetOem(CCmdUI *pCmdUI)
{
    if (pCmdUI->m_pSubMenu != NULL)
    {
        // This happens when popup menu itself is drawn
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION | (display_.char_area ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
    }
    else
    {
        pCmdUI->Enable(display_.char_area);
        pCmdUI->SetCheck(display_.char_set == CHARSET_OEM);
    }
}

void CHexEditView::OnCharsetEbcdic()
{
    bool std_scheme = false;

    if (!display_.char_area)
    {
        ASSERT(theApp.playing_);
        ::HMessageBox("You can't change characters sets without the char area");
        theApp.mac_error_ = 2;
        return;
    }

    // Change search type in find modeless dlg
    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();
    ASSERT(mm != NULL);
    if (display_.char_set != CHARSET_EBCDIC)
        mm->m_wndFind.m_pSheet->SetCharSet(CFindSheet::RB_CHARSET_EBCDIC);

    if (scheme_name_ == ASCII_NAME && display_.char_set == CHARSET_ASCII ||
        scheme_name_ == ANSI_NAME  && display_.char_set == CHARSET_ANSI  ||
        scheme_name_ == OEM_NAME   && display_.char_set == CHARSET_OEM )
    {
        std_scheme = true;
    }

    begin_change();
	display_.char_set = CHARSET_EBCDIC;
    BOOL ptoo = make_change();
    if (std_scheme)
    {
        undo_.push_back(view_undo(undo_scheme, ptoo));      // Allow undo of scheme change
        undo_.back().pscheme_name = new CString;
        *undo_.back().pscheme_name = scheme_name_;
        scheme_name_ = EBCDIC_NAME;
        set_colours();
    }
    end_change();

    theApp.SaveToMacro(km_charset, 3);
}

void CHexEditView::OnUpdateCharsetEbcdic(CCmdUI *pCmdUI)
{
    if (pCmdUI->m_pSubMenu != NULL)
    {
        // This happens when popup menu itself is drawn
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION | (display_.char_area ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
    }
    else
    {
        pCmdUI->Enable(display_.char_area);
        pCmdUI->SetCheck(display_.char_set == CHARSET_EBCDIC);
    }
}

void CHexEditView::OnControlNone()
{
    if (!display_.char_area || display_.char_set == CHARSET_EBCDIC)
    {
        ASSERT(theApp.playing_);
        if (!display_.char_area)
            ::HMessageBox("You can't display control characters without the char area");
        else if (display_.char_set == CHARSET_EBCDIC)
            ::HMessageBox("You can't display control characters in EBCDIC");
        theApp.mac_error_ = 2;
        return;
    }

    // Change the current state (storing previous state in undo vector if changed)
    begin_change();
    display_.control = 0;
    make_change();
    end_change();

    theApp.SaveToMacro(km_control, 1);
}

void CHexEditView::OnUpdateControlNone(CCmdUI *pCmdUI)
{
    if (pCmdUI->m_pSubMenu != NULL)
    {
        // This happens when popup menu itself is drawn
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION | 
            (display_.char_area && (display_.char_set == CHARSET_ASCII || display_.char_set == CHARSET_ANSI) ?
			        MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
    }
    else
    {
        pCmdUI->Enable(display_.char_area && (display_.char_set == CHARSET_ASCII || display_.char_set == CHARSET_ANSI));
        pCmdUI->SetCheck(display_.control == 0);
    }
}

void CHexEditView::OnControlAlpha()
{
    if (!display_.char_area || display_.char_set == CHARSET_EBCDIC)
    {
        ASSERT(theApp.playing_);
        if (!display_.char_area)
            ::HMessageBox("You can't display control characters without the char area");
        else if (display_.char_set == CHARSET_EBCDIC)
            ::HMessageBox("You can't display control characters in EBCDIC");
        theApp.mac_error_ = 2;
        return;
    }

    begin_change();
    display_.control = 1;
    make_change();
    end_change();

    theApp.SaveToMacro(km_control, 2);
}

void CHexEditView::OnUpdateControlAlpha(CCmdUI *pCmdUI)
{
    if (pCmdUI->m_pSubMenu != NULL)
    {
        // This happens when popup menu itself is drawn
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION | 
            (display_.char_area && (display_.char_set == CHARSET_ASCII || display_.char_set == CHARSET_ANSI) ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
    }
    else
    {
        pCmdUI->Enable(display_.char_area && (display_.char_set == CHARSET_ASCII || display_.char_set == CHARSET_ANSI));
        pCmdUI->SetCheck(display_.control == 1);
    }
}

void CHexEditView::OnControlC()
{
    if (!display_.char_area || display_.char_set == CHARSET_EBCDIC)
    {
        ASSERT(theApp.playing_);
        if (!display_.char_area)
            ::HMessageBox("You can't display control characters without the char area");
        else if (display_.char_set == CHARSET_EBCDIC)
            ::HMessageBox("You can't display control characters in EBCDIC");
        theApp.mac_error_ = 2;
        return;
    }

    begin_change();
    display_.control = 2;
    make_change();
    end_change();

    theApp.SaveToMacro(km_control, 3);
}

void CHexEditView::OnUpdateControlC(CCmdUI *pCmdUI)
{
    if (pCmdUI->m_pSubMenu != NULL)
    {
        // This happens when popup menu itself is drawn
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION | 
            (display_.char_area && (display_.char_set == CHARSET_ASCII || display_.char_set == CHARSET_ANSI) ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
    }
    else
    {
        pCmdUI->Enable(display_.char_area && (display_.char_set == CHARSET_ASCII || display_.char_set == CHARSET_ANSI));
        pCmdUI->SetCheck(display_.control == 2);
    }
}

void CHexEditView::OnAscEbc() 
{
    if (!display_.char_area)
    {
        // Can't display EBCDIC, presumably in macro playback
        ASSERT(theApp.playing_);
        ::HMessageBox("You can't display EBCDIC without the char area");
        theApp.mac_error_ = 2;
        return;
    }

    // Change search type in find modeless dlg
    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();
    ASSERT(mm != NULL);
    if (display_.char_set == CHARSET_EBCDIC)
        mm->m_wndFind.m_pSheet->SetCharSet(CFindSheet::RB_CHARSET_ASCII);
    else
        mm->m_wndFind.m_pSheet->SetCharSet(CFindSheet::RB_CHARSET_EBCDIC);

    begin_change();
	if (display_.char_set == CHARSET_EBCDIC)
		display_.char_set = CHARSET_ANSI;
	else
		display_.char_set = CHARSET_EBCDIC;
    make_change();
    end_change();

    theApp.SaveToMacro(km_ebcdic);
}

void CHexEditView::OnUpdateAscEbc(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(display_.char_area);
    pCmdUI->SetCheck(display_.char_set == CHARSET_EBCDIC);
}

void CHexEditView::OnControl() 
{
    CHECK_SECURITY(18);

    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
//    if (!aa->playing_ && GetFocus() != this) SetFocus(); // Ensure focus does not stay in DlgBar
    if (!display_.char_area || display_.char_set == CHARSET_EBCDIC)
    {
        // Can't toggle control chars, presumably in macro playback
        ASSERT(aa->playing_);
        if (!display_.char_area)
            ::HMessageBox("You can't display control characters without the char area");
        else
            ::HMessageBox("Control character display is not supported for EBCDIC");
        aa->mac_error_ = 2;
        return;
    }

    begin_change();
    display_.control = (display_.control + 1)%3;
    make_change();
    end_change();

    aa->SaveToMacro(km_control, unsigned __int64(0));
}

void CHexEditView::OnControlToggle() 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
    if (!display_.char_area || display_.char_set == CHARSET_EBCDIC)
    {
        // Can't toggle control chars, presumably in macro playback
        ASSERT(aa->playing_);
        if (!display_.char_area)
            ::HMessageBox("You can't display control characters without the char area");
        else
            ::HMessageBox("Control character display is not supported for EBCDIC");
        aa->mac_error_ = 2;
        return;
    }

    begin_change();
    // This is called as a result of menu item which has only 2 states (unlike
    // dialog bar button handled by OnControl() above which has 3)
    if (display_.control > 0)
        display_.control = 0;
    else
        display_.control = 1;
    make_change();
    end_change();

    aa->SaveToMacro(km_control, 99);
}

void CHexEditView::OnUpdateControl(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(display_.char_area && display_.char_set != CHARSET_EBCDIC);
    pCmdUI->SetCheck(display_.control != 0);
}

void CHexEditView::OnMark() 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
//    if (!aa->playing_ && GetFocus() != this) SetFocus(); // Ensure focus does not stay in DlgBar

    SetMark(GetPos());

    // Keep track whether mark is in hex or char area (if char area on)
    if (display_.vert_display)
    {
        // If cursor is in char row set mark to be in char area
        if (pos2row(GetCaret()) == 0)
            display_.mark_char = TRUE;
        else
            display_.mark_char = FALSE;
    }
    else if (display_.char_area && display_.hex_area)
        display_.mark_char = display_.edit_char;
    else if (display_.char_area)
        display_.mark_char = TRUE;
    else if (display_.hex_area)
        display_.mark_char = FALSE;

    aa->SaveToMacro(km_mark_pos);
}

void CHexEditView::SetMark(FILE_ADDRESS new_mark)
{
    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    // Invalidate where mark was to change colours
    invalidate_addr_range(mark_, mark_ + 1);

    // Save current mark and move mark_
    undo_.push_back(view_undo(undo_setmark));
    undo_.back().address = mark_;
    undo_.push_back(view_undo(undo_state, TRUE));
    undo_.back().disp_state = disp_state_;
    mark_ = new_mark;
	if (theApp.align_mark_ && mark_ != GetDocument()->base_addr_)
	{
		GetDocument()->StopSearch();
		GetDocument()->base_addr_ = mark_;
		GetDocument()->StartSearch();
	}

    // Invalidate where mark now is to change background colour
    invalidate_addr_range(mark_, mark_ + 1);

    CHECK_SECURITY(41);

    show_calc();                        // Some button enablement depends on mark_ position (eg. @ Mark)
}

void CHexEditView::OnGotoMark() 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
//    if (!aa->playing_ && GetFocus() != this) SetFocus(); // Ensure focus does not stay in DlgBar
    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    if (!display_.vert_display && 
        display_.char_area && display_.hex_area && 
        display_.edit_char != display_.mark_char)
    {
        // Going to mark also entails swapping between hex and char areas
        FILE_ADDRESS start_addr, end_addr;
        GetSelAddr(start_addr, end_addr);
        int row = 0;
        if (start_addr == end_addr && display_.vert_display)
            row = pos2row(GetCaret());

        undo_.push_back(view_undo(undo_state));
        undo_.back().disp_state = disp_state_;

        display_.edit_char = display_.mark_char;
        if (display_.edit_char)
            SetHorzBufferZone(1);
        else
            SetHorzBufferZone(2);
        // We need to specify the previous address since otherwise MoveToAddress
        // will use the current selection which is now in the wrong area.
        MoveWithDesc("Jump to Mark ", mark_, mark_, start_addr, end_addr, TRUE);  // space at end means significant nav pt

        // Need to call SetSel in case MoveToAddress did not, due to no move (only area swap)
        SetSel(addr2pos(mark_, row), addr2pos(mark_, row), true);
    }
    else
		MoveWithDesc("Jump to Mark ", mark_);  // space at end means significant nav pt

    aa->SaveToMacro(km_goto_mark);
}

void CHexEditView::OnExtendToMark() 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
//    if (!aa->playing_ && GetFocus() != this) SetFocus(); // Ensure focus does not stay in DlgBar
    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    FILE_ADDRESS start_addr, end_addr;
    BOOL end_base = GetSelAddr(start_addr, end_addr);
    if ((end_base && mark_ == end_addr) ||
        (!end_base && mark_ == start_addr))
    {
        // There is nothing to do
        aa->mac_error_ = 1;
        return;
    }

    // Move the non-base end of the selection to the mark (MoveToAddress saves undo info)
    if (end_base)
        MoveToAddress(mark_, end_addr);
    else
        MoveWithDesc("Extend to Mark ", start_addr, mark_);  // space at end means significant nav pt

    aa->SaveToMacro(km_extendto_mark);
}

// Swap the current caret position with the mark
void CHexEditView::OnSwapMark() 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
//    if (!aa->playing_ && GetFocus() != this) SetFocus(); // Ensure focus does not stay in DlgBar
    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

    FILE_ADDRESS start_addr, end_addr;
    BOOL end_base = GetSelAddr(start_addr, end_addr);
    int row = 0;
    if (start_addr == end_addr && display_.vert_display)
        row = pos2row(GetCaret());

    // If same address and mark/caret are in same areas (both in hex or both in char area) return
    if (mark_ == start_addr && start_addr == end_addr && 
        (display_.vert_display || !display_.char_area || !display_.hex_area || display_.edit_char == display_.mark_char))
    {
        // There is nothing to do
        aa->mac_error_ = 1;
        return;
    }

    // If the caret and the mark are in different areas we have to swap them
    if (!display_.vert_display && 
        display_.char_area && display_.hex_area && 
        display_.edit_char != display_.mark_char)
    {
        undo_.push_back(view_undo(undo_state));        // save undo for edit_char_ and mark_char_
        undo_.back().disp_state = disp_state_;
        display_.edit_char = !display_.edit_char;
        display_.mark_char = !display_.mark_char;
        if (display_.edit_char)
            SetHorzBufferZone(1);
        else
            SetHorzBufferZone(2);

        // We need to specify the previous address since otherwise MoveToAddress
        // will use the current selection which is now in the wrong area since
        // display_.edit_char has changed.
        MoveWithDesc("Swap Cursor with Mark", mark_, mark_, start_addr, end_addr, TRUE);

        // Need to call SetSel in case MoveToAddress did not due to no move (only area swap)
        SetSel(addr2pos(mark_), addr2pos(mark_), true);
    }
    else
        MoveWithDesc("Swap Cursor with Mark", mark_, -1, -1, -1, FALSE, FALSE, row);

    // Move the mark
    undo_.push_back(view_undo(undo_setmark, TRUE));     // save undo for move mark
    undo_.back().address = mark_;
    invalidate_addr_range(mark_, mark_ + 1);            // force undraw of mark
    mark_ = start_addr;
    invalidate_addr_range(mark_, mark_ + 1);            // force draw of new mark
	if (theApp.align_mark_ && mark_ != GetDocument()->base_addr_)
	{
		GetDocument()->StopSearch();
		GetDocument()->base_addr_ = mark_;
		GetDocument()->StartSearch();
	}
    show_calc();

    aa->SaveToMacro(km_swap_mark);
}

void CHexEditView::OnJumpDec()           // message from BCG edit bar combo
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();

    CString addr_str, err_str;
    FILE_ADDRESS address = mm->GetDecAddress(addr_str, err_str);

    if (address == -1)
    {
        CString ss;
        ss.Format("Invalid expression\r\r%s\r\r%s", addr_str, err_str);
        AfxMessageBox(ss);
        return;
    }

    // If recording macro indicate jump & store address jumped to
    aa->SaveToMacro(km_address_dec, addr_str);

#if 0
    // Try to go to the address requested
    FILE_ADDRESS actual;
    if ((actual = GoAddress(address)) != address)
    {
        // Could not go to the requested address - tell user
        ::HMessageBox("Invalid address entered. Address set to EOF");
    }
    SaveMove();          // Save prev pos in undo array
    DisplayCaret();
#else
    MoveWithDesc("Jump (Decimal Jump Tool) ", address);  // space at end means significant nav pt
#endif
}

void CHexEditView::OnJumpHex()           // message from BCG edit bar combo
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();

    CString addr_str, err_str;
    FILE_ADDRESS address = mm->GetHexAddress(addr_str, err_str);

    if (address == -1)
    {
        CString ss;
        ss.Format("Invalid hex expression\r\r%s\r\r%s", addr_str, err_str);
        AfxMessageBox(ss);
        return;
    }

    // If recording macro indicate jump & store address jumped to
    aa->SaveToMacro(km_address_hex, addr_str);

#if 0
    // Try to go to the address requested
    FILE_ADDRESS actual;
    if ((actual = GoAddress(address)) != address)
    {
        // Could not go to the requested address - tell user
        ::HMessageBox("Invalid address entered. Address set to EOF");
    }
    SaveMove();          // Save prev pos in undo array
    DisplayCaret();
#else
    MoveWithDesc("Jump (Hex Jump Tool) ", address);  // space at end means significant nav pt
#endif
}

void CHexEditView::OnJumpHexAddr()      // Alt-J
{
    num_entered_ = num_del_ = num_bs_ = 0;      // Stop any editing

#if 0 // BCG toolbar changes
    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();
    if ((mm->m_wndEditBar.GetStyle() & WS_VISIBLE) != 0)
        mm->m_wndEditBar.GetDlgItem(IDC_JUMP_HEX)->SetFocus();
#else
    CObList listButtons;

    // Search all toolbars and jump to the first visible hex address tool found (can be more than one)
    if (CBCGToolBar::GetCommandButtons(ID_JUMP_HEX_COMBO, listButtons) > 0)
    {
        for (POSITION posCombo = listButtons.GetHeadPosition(); 
            posCombo != NULL; )
        {
            CHexComboButton* pCombo = 
                DYNAMIC_DOWNCAST(CHexComboButton, listButtons.GetNext(posCombo));
            ASSERT(pCombo != NULL);

            CHexEditControl *pedit = pCombo->GetEdit();
            ASSERT(pedit != NULL);
            if (pedit->IsWindowVisible())
            {
                pedit->SetFocus();
                break;
            }
        }
    }
#endif
}

void CHexEditView::OnInsert()
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
//    if (!aa->playing_ && GetFocus() != this) SetFocus(); // Ensure focus does not stay in DlgBar
    num_entered_ = num_del_ = num_bs_ = 0;

	if (!do_insert())
		return;

    aa->SaveToMacro(km_ovr_ins);
}

bool CHexEditView::do_insert()
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());

    if (display_.readonly)
    {
        ::HMessageBox("Attempt to toggle OVR/INS in read only mode");
        aa->mac_error_ = 10;
        return false;
    }
	if (GetDocument()->IsDevice())
    {
        ::HMessageBox("You cannot use INS mode for devices (logical volumes and physical disks)");
        aa->mac_error_ = 10;
        return false;
    }
    display_.overtype = !display_.overtype;
    undo_.push_back(view_undo(undo_overtype));
    undo_.back().flag = !display_.overtype;
    if (GetDocument()->length() == 0)
    {
        DoInvalidate();
        if (display_.overtype)
            ScrollMode();
        else
        {
            CaretMode();
            SetCaret(addr2pos(0));  // very start of file
        }
    }
	return true;
}

void CHexEditView::OnUpdateInsert(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(!display_.readonly && !GetDocument()->IsDevice());
    pCmdUI->SetCheck(!display_.overtype);
}

void CHexEditView::OnAllowMods() 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
//    if (!aa->playing_ && GetFocus() != this) SetFocus(); // Ensure focus does not stay in DlgBar
    num_entered_ = num_del_ = num_bs_ = 0;

    allow_mods();

    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();

    // Make sure calc buttons reflect modifiability of file
    if (!aa->refresh_off_ && mm->m_wndCalc.IsVisible())
        mm->m_wndCalc.update_controls();

#if 0 // Obviated by BCG
    // Change the toolbar button
    mm->bb_allow_mods_.LoadBitmaps(display_.readonly ? IDB_MODSU : IDB_MODSS,
                                    IDB_MODSD,0,IDB_MODSX);
    mm->bb_allow_mods_.Invalidate();
#endif
    aa->SaveToMacro(km_ro_rw);
}

void CHexEditView::allow_mods() 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());

    if (display_.readonly && GetDocument()->read_only())
    {
        ::HMessageBox("This file cannot be modified");
        aa->mac_error_ = 10;
        return;
    }
    display_.readonly = !display_.readonly;
    undo_.push_back(view_undo(undo_readonly));
    undo_.back().flag = !display_.readonly;
    show_prop();  // things may be changeable now
}

void CHexEditView::OnUpdateAllowMods(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(!GetDocument()->read_only());
    pCmdUI->SetCheck(!display_.readonly);
}

void CHexEditView::OnToggleEndian() 
{
    begin_change();
    display_.big_endian = !display_.big_endian;
    make_change();
    end_change();

    // Make calculator big endian check box reflects this file's big endian status
    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();
    if (!theApp.refresh_off_ && mm->m_wndCalc.IsVisible())
        mm->m_wndCalc.update_controls();

    theApp.SaveToMacro(km_big_endian, (__int64)-1);  // -1 = toggle
}

void CHexEditView::OnUpdateToggleEndian(CCmdUI* pCmdUI) 
{
    pCmdUI->SetCheck(display_.big_endian);
}

void CHexEditView::OnBigEndian() 
{
    begin_change();
    display_.big_endian = 1;
    make_change();
    end_change();

    // Make calculator big endian check box reflects this file's big endian status
    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();
    if (!theApp.refresh_off_ && mm->m_wndCalc.IsVisible())
        mm->m_wndCalc.update_controls();

    theApp.SaveToMacro(km_big_endian, (__int64)1);  // 1 = big endian on
}

void CHexEditView::OnUpdateBigEndian(CCmdUI* pCmdUI) 
{
    pCmdUI->SetCheck(display_.big_endian);
}

void CHexEditView::OnLittleEndian() 
{
    begin_change();
    display_.big_endian = 0;
    make_change();
    end_change();

    // Make calculator big endian check box reflects this file's big endian status
    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();
    if (!theApp.refresh_off_ && mm->m_wndCalc.IsVisible())
        mm->m_wndCalc.update_controls();

    theApp.SaveToMacro(km_big_endian, unsigned __int64(0));  // 0 = big endian off (ie little-endian)
}

void CHexEditView::OnUpdateLittleEndian(CCmdUI* pCmdUI) 
{
    pCmdUI->SetCheck(!display_.big_endian);
}

void CHexEditView::OnTrackChanges() 
{
    begin_change();
    if (display_.hide_replace && display_.hide_insert && display_.hide_delete)
    {
        // All off - so turn them all on
        display_.hide_replace = display_.hide_insert = display_.hide_delete = 0;
    }
    else
    {
        // Turn them all off
        display_.hide_replace = display_.hide_insert = display_.hide_delete = 1;
    }
    make_change();
    end_change();
    theApp.SaveToMacro(km_track_changes, (__int64)-1);  // -1 = toggle
}

void CHexEditView::OnUpdateTrackChanges(CCmdUI* pCmdUI) 
{
    pCmdUI->SetCheck(!(display_.hide_replace && display_.hide_insert && display_.hide_delete));
}

void CHexEditView::OnDffdAutoSync() 
{
    if (pdfv_ == NULL)
    {
        ::HMessageBox("No DFFD tree view is displayed");
        theApp.mac_error_ = 10;
        return;
    }
    begin_change();
    display_.auto_sync = !display_.auto_sync;
    make_change();
    end_change();

    // If has been turned on then sync now
    if (display_.auto_sync)
    {
        FILE_ADDRESS start_addr, end_addr;
        GetSelAddr(start_addr, end_addr);
        pdfv_->SelectAt(start_addr);
    }

    theApp.SaveToMacro(km_dffd_sync, 2);
}

void CHexEditView::OnUpdateDffdAutoSync(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(pdfv_ != NULL);
    pCmdUI->SetCheck(display_.auto_sync);
}

void CHexEditView::OnDffdSync() 
{
    if (pdfv_ == NULL)
    {
        ::HMessageBox("No DFFD tree view is displayed");
        theApp.mac_error_ = 10;
        return;
    }

    FILE_ADDRESS start_addr, end_addr;
    GetSelAddr(start_addr, end_addr);
    pdfv_->SelectAt(start_addr);
    pdfv_->SetFocus();
    theApp.SaveToMacro(km_dffd_sync, 255);
}

void CHexEditView::OnUpdateDffdSync(CCmdUI* pCmdUI) 
{
    // Don't allow manual sync if auto sync is on
    pCmdUI->Enable(pdfv_ != NULL && !display_.auto_sync);
}

void CHexEditView::OnSearchHex()        // Alt-L, F6
{
    CObList listButtons;

    // Search all toolbars and jump to the first visible search tool found (can be more than one)
    if (CBCGToolBar::GetCommandButtons(ID_SEARCH_COMBO, listButtons) > 0)
    {
        for (POSITION posCombo = listButtons.GetHeadPosition(); 
            posCombo != NULL; )
        {
            CFindComboButton* pCombo = 
                DYNAMIC_DOWNCAST(CFindComboButton, listButtons.GetNext(posCombo));
            ASSERT(pCombo != NULL);

            CSearchEditControl *pedit = pCombo->GetEdit();
            ASSERT(pedit != NULL);
            if (pedit->IsWindowVisible())
            {
                pedit->SetMode(CSearchEditControl::mode_hex);
                pedit->SetFocus();
                break;
            }
        }
    }
}

void CHexEditView::OnSearchAscii()      // F5
{
    CObList listButtons;

    // Search all toolbars and jump to the first visible search tool found (can be more than one)
    if (CBCGToolBar::GetCommandButtons(ID_SEARCH_COMBO, listButtons) > 0)
    {
        for (POSITION posCombo = listButtons.GetHeadPosition(); 
            posCombo != NULL; )
        {
            CFindComboButton* pCombo = 
                DYNAMIC_DOWNCAST(CFindComboButton, listButtons.GetNext(posCombo));
            ASSERT(pCombo != NULL);

            CSearchEditControl *pedit = pCombo->GetEdit();
            ASSERT(pedit != NULL);
            if (pedit->IsWindowVisible())
            {
                pedit->SetMode(CSearchEditControl::mode_char);
                pedit->SetFocus();
                break;
            }
        }
    }
}

void CHexEditView::OnSearchIcase()      // F4
{
    CObList listButtons;

    // Search all toolbars and jump to the first visible search tool found (can be more than one)
    if (CBCGToolBar::GetCommandButtons(ID_SEARCH_COMBO, listButtons) > 0)
    {
        for (POSITION posCombo = listButtons.GetHeadPosition(); 
            posCombo != NULL; )
        {
            CFindComboButton* pCombo = 
                DYNAMIC_DOWNCAST(CFindComboButton, listButtons.GetNext(posCombo));
            ASSERT(pCombo != NULL);

            CSearchEditControl *pedit = pCombo->GetEdit();
            ASSERT(pedit != NULL);
            if (pedit->IsWindowVisible())
            {
                pedit->SetMode(CSearchEditControl::mode_icase);
                pedit->SetFocus();
                break;
            }
        }
    }
}

// 
CChildFrame *CHexEditView::comp_window()
{
    CMainFrame *mm = dynamic_cast<CMainFrame *>(AfxGetMainWnd());
    CChildFrame *nextc;         // Loops through all MDI child frames
    CChildFrame *compc = NULL;  // Frame of view available to compare with
    BOOL got_one;               // Have we gound an appropriate view?

    // Get the currently active child MDI window
    nextc = dynamic_cast<CChildFrame *>(mm->MDIGetActive());
    ASSERT(nextc != NULL);
    ASSERT(nextc->GetHexEditView() == this);

    // Search for another (non-iconized) window
    for (got_one = FALSE, nextc = dynamic_cast<CChildFrame *>(nextc->GetWindow(GW_HWNDNEXT));
         nextc != NULL; nextc = dynamic_cast<CChildFrame *>(nextc->GetWindow(GW_HWNDNEXT)) )
    {
        if (!nextc->IsIconic())
        {
            // More than one found - use which one?
            if (got_one)
            {
// Can't display message when used in OnUpdateEditCompare call
//              ::HMessageBox("Comparison not performed\r"
//                              "- more than two (non-iconized) windows");
                return NULL;
            }
            got_one = TRUE;
            compc = nextc;
        }
    }

    // If we didn't find a non-iconized window search for iconized one
    if (compc == NULL)
    {
        nextc = dynamic_cast<CChildFrame *>(mm->MDIGetActive());

        // Search for an iconized window
        for (got_one = FALSE, nextc = dynamic_cast<CChildFrame *>(nextc->GetWindow(GW_HWNDNEXT));
             nextc != NULL; nextc = dynamic_cast<CChildFrame *>(nextc->GetWindow(GW_HWNDNEXT)) )
        {
            ASSERT(nextc->IsIconic());          // else compc != NULL
            // If more than one window - use which one?
            if (got_one)
            {
// Can't display message when used in OnUpdateEditCompare call
//              ::HMessageBox("Comparison not performed\r"
//                              "- more than two windows found");
                return NULL;
            }
            got_one = TRUE;
            compc = nextc;
        }
    }
    return compc;
}

void CHexEditView::OnEditCompare() 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
    CMainFrame *mm = dynamic_cast<CMainFrame *>(AfxGetMainWnd());
    CChildFrame *origc;        // Pointer to MDI child frame of current view

    // Save the currently active child MDI window
    origc = dynamic_cast<CChildFrame *>(mm->MDIGetActive());
    ASSERT(origc != NULL);
    ASSERT(origc->GetHexEditView() == this);

    // Restore current window if iconized
    if (origc->IsIconic())
    {
        WINDOWPLACEMENT wp;
        origc->GetWindowPlacement(&wp);
        wp.showCmd = SW_RESTORE;
        origc->SetWindowPlacement(&wp);
    }

    // Get MDI child frame of compare window
    CChildFrame *compc = comp_window();

    // If we found nothing to compare with, display message and return
    if (compc == NULL)
    {
        ::HMessageBox("Comparison not performed\r"
                        "- two non-minimized windows required");
        aa->mac_error_ = 10;
        return;
    }

    // Get view to compare with - active view of compc.  (Actually the
    // active view should be the only view since we don't have splitters.)
    CHexEditView *compv = dynamic_cast<CHexEditView *>(compc->GetHexEditView());
    if (compv == NULL)
    {
        ::HMessageBox("Cannot compare with this type of window");
        aa->mac_error_ = 10;
        return;
    }
    ASSERT_KINDOF(CHexEditView, compv);

    CString orig_title, comp_title;     // Title of the windows to be compared
    origc->GetWindowText(orig_title);
    compc->GetWindowText(comp_title);

    CHexEditDoc *origd, *compd;         // Documents of the compared views
    origd = GetDocument();
    compd = compv->GetDocument();

    // Now compare the data from each view starting at END of current selection
    CString mess;                       // Message for user when problem encountered

    FILE_ADDRESS dummy;                         // Not used - start address of selection
    FILE_ADDRESS start_addr;                    // Start address in current view
    FILE_ADDRESS orig_addr, comp_addr;          // Current comp location in both views
    GetSelAddr(dummy, start_addr);
    orig_addr = start_addr;
    compv->GetSelAddr(dummy, comp_addr);
//    start_addr = orig_addr = GetPos();
//    comp_addr = compv->pos2addr(compv->GetCaret());

#ifndef _DEBUG  // Allow self-compare for testing purposes
    // If same doc and same address then we aren't doing anything useful
    if (origd == compd && orig_addr == comp_addr)
    {
        mess.Format("Comparing data with itself in windows\r%s and %s",
                        (const char *)orig_title, (const char *)comp_title);
        ::HMessageBox(mess);
        aa->mac_error_ = 10;
        return;
    }
#endif

    size_t orig_got, comp_got;          // How many bytes obtained from docs

    FILE_ADDRESS show_inc = 0x80000;            // How far between showing addresses
    FILE_ADDRESS next_show = (orig_addr/show_inc + 1)*show_inc; // Next address to show
    FILE_ADDRESS slow_show = ((orig_addr+0x800000)/0x800000 + 1)*0x800000;      // When we slow showing
    FILE_ADDRESS comp_length = min(origd->length() - orig_addr, compd->length() - comp_addr);
    bool progress_on = false;           // Are we showing progress in the status bar?

    // Get memory for compare buffers
    unsigned char *orig_buf = new unsigned char[compare_buf_len];
    unsigned char *comp_buf = new unsigned char[compare_buf_len];

    CWaitCursor wait;
    aa->SaveToMacro(km_compare);

    while (1)
    {
        if (orig_addr > next_show)
        {
            MSG msg;

            // Do any redrawing, but nothing else
            while (::PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_NOREMOVE))
            {
                if (::GetMessage(&msg, NULL, WM_PAINT, WM_PAINT))
                {
                    ::TranslateMessage(&msg);
                    ::DispatchMessage(&msg);
                }
            }

            // Check if a key has been pressed
            if (::PeekMessage(&msg, NULL, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE))
            {
                // Windows does not like to miss key down events (need to match key up events)
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);

                // Remove any characters resulting from keypresses (so they are not inserted into the active file)
                while (::PeekMessage(&msg, NULL, WM_CHAR, WM_CHAR, PM_REMOVE))
                    ;

                if (msg.wParam != 'y' &&  // avoid accidental multiple presses of Y key from aborting
                    ::HMessageBox("Abort comparison?", MB_YESNO) == IDYES)
                {
                    delete[] orig_buf;
                    delete[] comp_buf;
                    if (progress_on)
                        mm->m_wndStatusBar.EnablePaneProgressBar(0, -1);
                    ((CMainFrame *)AfxGetMainWnd())
                        ->StatusBarText("Comparison aborted");
                    aa->mac_error_ = 10;
                    show_pos();
                    return;
                }
            }

            show_pos(next_show);

            // If we've been showing the current address for awhile then show
            // less often (and show progress) to avoid slowing down the actual compare
            if (next_show >= slow_show)
            {
                if (!progress_on)
                {
                    progress_on = true;
                    mm->m_wndStatusBar.EnablePaneProgressBar(0, 100);
                }
                mm->m_wndStatusBar.SetPaneProgress(0, long(((next_show-start_addr)*100)/comp_length));
                show_inc = 0x800000;
            }
            AfxGetApp()->OnIdle(0);         // Force display of updated address
            next_show += show_inc;
        }

        orig_got = origd->GetData(orig_buf, compare_buf_len, orig_addr);
        comp_got = compd->GetData(comp_buf, compare_buf_len, comp_addr);

        size_t comp_len = min(orig_got, comp_got);
        if (comp_len == 0)              // EOF of one or both files
            break;

        if (memcmp(orig_buf, comp_buf, comp_len) != 0)
        {
            // Difference found
            for (size_t pos = 0; pos < comp_len; ++pos)
                if (orig_buf[pos] != comp_buf[pos])
                    break;
            ASSERT(pos < comp_len);

#ifdef SYS_SOUNDS
            CSystemSound::Play("Comparison Difference Found");
#endif

//          mess.Format("Difference found after $%I64X (decimal %I64d) bytes\r"
//                      "%s at address $%I64X (decimal %I64d)\r%s at address $%I64X (decimal %I64d)",
//                      __int64(orig_addr + pos - start_addr), __int64(orig_addr + pos - start_addr),
//                      orig_title, __int64(orig_addr + pos), __int64(orig_addr + pos),
//                      comp_title, __int64(comp_addr + pos), __int64(comp_addr + pos));
//          ::HMessageBox(mess);
            if (progress_on)
                mm->m_wndStatusBar.EnablePaneProgressBar(0, -1);
            char buf[32];
            sprintf(buf, "%I64d", __int64(orig_addr + pos - start_addr));
            mess = CString("Difference found after ") + buf + CString(" bytes");
            mm->StatusBarText(mess);

            // Move to where diff found and select that byte in both views
            // (Selecting the byte allows the differences to be seen in both
            // windows without flipping between them, and allows another
            // compare immediately starting at the byte after.)
            compv->MoveWithDesc("Comparison Difference Found ", comp_addr + pos, comp_addr + pos + 1);
            MoveWithDesc("Comparison Difference Found ", orig_addr + pos, orig_addr + pos + 1);
            if (aa->highlight_)
            {
                compv->add_highlight(comp_addr + pos, comp_addr + pos + 1, TRUE);
                add_highlight(orig_addr + pos, orig_addr + pos + 1, TRUE);
            }
            delete[] orig_buf;
            delete[] comp_buf;
            return;
        }

        if (orig_got != comp_got)
            break;

        orig_addr += orig_got;
        comp_addr += comp_got;
    }

    if (progress_on)
        mm->m_wndStatusBar.EnablePaneProgressBar(0, -1);

    // If we got here then we hit EOF on one or both files
    if (orig_got == comp_got && orig_addr + orig_got - start_addr == 0)
    {
#ifdef SYS_SOUNDS
        CSystemSound::Play("Comparison Difference Not Found");
#endif
        mess = "Both files are at EOF";
        if (aa->playing_)
            mm->StatusBarText(mess);
        else
            ::HMessageBox(mess);
    }
    else if (orig_got == comp_got)
    {
#ifdef SYS_SOUNDS
        CSystemSound::Play("Comparison Difference Not Found");
#endif
        // EOF (both files) encountered
        ASSERT(orig_got == 0);
        show_pos(orig_addr);

        // Display message box when no differences found so that user sees that
        // something happened -- the caret is not moved and they may not notice
        // a message in the status bar (or the status bar may be invisible).
        // (On the other hand if a difference is found then the caret of both views
        // is moved and we just display a message in the status bar -- putting up 
        // a dialog would just be annoying.)

        CString sdec, shex;
        char buf[22];                    // temp buf where we sprintf
        sprintf(buf, "%I64d", __int64(orig_addr + orig_got - start_addr));
        sdec = buf;
        AddCommas(sdec);

        if (aa->hex_ucase_)
            sprintf(buf, "%I64X", __int64(orig_addr + orig_got - start_addr));
        else
            sprintf(buf, "%I64x", __int64(orig_addr + orig_got - start_addr));
        shex = buf;
        AddSpaces(shex);

        mess.Format("No differences found after\r"
                    "%s (%sh) bytes.", sdec, shex);
//        mess.Format("No differences found\r"
//                    "after %lX (hex) or\r"
//                    "%ld (decimal) bytes.",
//                    orig_addr + orig_got - start_addr,
//                    orig_addr + orig_got - start_addr);
        if (aa->playing_)
            mm->StatusBarText(mess);
        else
            ::HMessageBox(mess);
        aa->mac_error_ = 1;
    }
    else if (orig_got < comp_got)
    {
#ifdef SYS_SOUNDS
        CSystemSound::Play("Comparison Difference Found");
#endif
        // EOF on orig file before EOF on comp file
//      mess.Format("Difference found after $%I64X (decimal %I64d) bytes\r"
//                  "%s at EOF - address $%I64X (decimal %I64d)\r%s at address $%I64X (decimal %I64d)",
//                  __int64(orig_addr + orig_got - start_addr), __int64(orig_addr + orig_got - start_addr),
//                  orig_title, __int64(orig_addr + orig_got), __int64(orig_addr + orig_got),
//                  comp_title, __int64(comp_addr + orig_got), __int64(comp_addr + orig_got));
//      ::HMessageBox(mess);
        char buf[32];
        sprintf(buf, "%I64d", __int64(orig_addr + orig_got - start_addr));
        mess.Format("EOF on \"%s\" after %s bytes", orig_title, buf);
        mm->StatusBarText(mess);

        compv->MoveWithDesc("Comparison Difference Found ", comp_addr + orig_got, comp_addr + orig_got + 1);
        if (aa->highlight_)
            compv->add_highlight(comp_addr + orig_got, comp_addr + orig_got + 1, TRUE);
        MoveWithDesc("EOF in Comparison ", orig_addr + orig_got);
    }
    else
    {
#ifdef SYS_SOUNDS
        CSystemSound::Play("Comparison Difference Found");
#endif
        // EOF on comp file before EOF on orig file
//      mess.Format("Difference found after $%lX (decimal %ld) bytes\r"
//                  "%s at address $%lX (decimal %ld)\r%s at EOF - address $%lX (decimal %ld)",
//                  orig_addr + comp_got - start_addr, orig_addr + comp_got - start_addr,
//                  orig_title, orig_addr + comp_got, orig_addr + comp_got,
//                  comp_title, comp_addr + comp_got, comp_addr + comp_got);
//      ::HMessageBox(mess);
        char buf[32];
        sprintf(buf, "%I64d", __int64(orig_addr + comp_got - start_addr));
        mess.Format("EOF on \"%s\" after %s bytes", comp_title, buf);
        mm->StatusBarText(mess);

        compv->MoveWithDesc("EOF in Comparison ", comp_addr + comp_got);
        MoveWithDesc("Comparison Difference Found ", orig_addr + comp_got, orig_addr + comp_got + 1);
        if (aa->highlight_)
            add_highlight(orig_addr + comp_got, orig_addr + comp_got + 1, TRUE);
    }
    delete[] orig_buf;
    delete[] comp_buf;
}

void CHexEditView::OnUpdateEditCompare(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(comp_window() != NULL);
}

// Activate next window
void CHexEditView::OnWindowNext()
{
#if 0
// This was changed for consistency with km_win_next in macros.
// The problem is to make sure all windows are cycled through when
// Window/Next is used whether or not in a macro (and in the same order).
// - next window in macro does not change focus now (for speed) but just keeps track of active view
// - if focus is set Windows changes order of windows (Z order) returned by GetWindow(GW_HWNDNEXT)
// - the MDI functions (MDINext etc) seem to use some sort of list where order does not change with focus change
// - but MDINext changes focus and there appears to be no way to get at the list without changing focus
// For the above reasons and for consistency the next window command both
// in and not in macros uses the new NextView function below.

    // Activate next window that is not a print preview window and is not minimized
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
    CMainFrame *mm = dynamic_cast<CMainFrame *>(::AfxGetMainWnd());
    CChildFrame *currc;        // Currently active MDI child frame
    CChildFrame *nextc;        // Loops through all MDI child frames

    // Save the currently active child MDI window
    currc = dynamic_cast<CChildFrame *>(mm->MDIGetActive());
    ASSERT(currc != NULL);
    ASSERT(currc->GetHexEditView() == this);

    while (mm->MDINext(), (nextc = dynamic_cast<CChildFrame *>(mm->MDIGetActive())) != currc)
    {
        // Don't change to iconized windows
        if (!nextc->IsIconic())
        {
            // Make sure it's a CHexEditView (don't change to print preview windows)
            CHexEditView *pview = dynamic_cast<CHexEditView *>(nextc->GetHexEditView());
            if (pview != NULL && pview->IsKindOf(RUNTIME_CLASS(CHexEditView)))
            {
                nextc->SetActiveView(pview);
                if (aa->recording_ && aa->mac_.size() > 0 && (aa->mac_.back()).ktype == km_focus)
                {
                    // We don't want focus change recorded (see CHexEditView::OnSetFocus)
                    aa->mac_.pop_back();
                }
                aa->SaveToMacro(km_win_next);
                return;
            }
        }
    }
    ((CMainFrame *)AfxGetMainWnd())->
        StatusBarText("Warning: no other non-minimized windows found");
    aa->mac_error_ = 2;
#endif

    // Get the next window
    CHexEditView *pnext = NextView();

    if (pnext == NULL)
    {
        ::HMessageBox("Warning: no other non-minimized windows found");
        theApp.mac_error_ = 2;
        return;
    }

    // Make new view active
	pnext->GetFrame()->MDIActivate();           // Activate next frame
    pnext->GetFrame()->SetActiveView(pnext);    // Make sure active view is normal (hex) view

	ASSERT(pnext == GetView());       // Make sure it is now the active view
    if (theApp.recording_ && theApp.mac_.size() > 0 && (theApp.mac_.back()).ktype == km_focus)
    {
        // We don't want focus change recorded (see CHexEditView::OnSetFocus)
        theApp.mac_.pop_back();
    }
    theApp.SaveToMacro(km_win_next);
}

// Return next view in list non-minimised view or NULL if none.
// Cycles through the list in reverse order so repeated calls
// and focus changes still return all the non-minimised frames.
CHexEditView * CHexEditView::NextView() const
{
	CChildFrame *currc = dynamic_cast<CChildFrame *>(GetFrame());
	ASSERT(currc != NULL);
    CChildFrame *nextc = currc;

    do
    {
        nextc = dynamic_cast<CChildFrame *>(nextc->GetWindow(GW_HWNDPREV));
        // If reached the top of the list go to the end
        if (nextc == NULL)
            nextc = dynamic_cast<CChildFrame *>(currc->GetWindow(GW_HWNDLAST));
    } while (nextc != NULL && nextc != currc && nextc->IsIconic());

    if (nextc == NULL || nextc == currc)
        return NULL;
    else
        return nextc->GetHexEditView();
}

void CHexEditView::OnAscii2Ebcdic() 
{
	DoConversion(CONVERT_ASC2EBC, "convert ASCII to EBCDIC");
}

void CHexEditView::OnEbcdic2Ascii() 
{
    DoConversion(CONVERT_EBC2ASC, "convert EBCDIC to ASCII");
}

void CHexEditView::OnAnsi2Ibm() 
{
    DoConversion(CONVERT_ANSI2IBM, "convert ANSI to IBM/OEM");
}

void CHexEditView::OnIbm2Ansi() 
{
    DoConversion(CONVERT_IBM2ANSI, "convert IBM/OEM to ANSI");
}

void CHexEditView::OnUppercase() 
{
    DoConversion(CONVERT_UPPER, "convert to upper case");
}

void CHexEditView::OnLowercase() 
{
    DoConversion(CONVERT_LOWER, "convert to lower case");
}

// This handles conversion operations (everything except for the actual data
// convert operation - see ProcConversion) including getting the selection
// and allocating memory buffers to store the result (or temp file for
// really big selections) and updating the document.
// It also updates progress in the status bar and allows user abort.
void CHexEditView::DoConversion(convert_type op, LPCSTR desc)
{
    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();
    unsigned char *buf = NULL;

    if (check_ro(desc))
        return;

    // Get current address or selection
    FILE_ADDRESS start_addr, end_addr;          // Start and end of selection
    GetSelAddr(start_addr, end_addr);

    // Make sure there is a selection
    if (start_addr >= end_addr)
    {
		ASSERT(theApp.playing_);
		CString mess;
		mess.Format("There is no selection to %s", desc);
		::HMessageBox(mess);
		theApp.mac_error_ = 10;
		return;
    }
    ASSERT(start_addr < end_addr && end_addr <= GetDocument()->length());

	// Test if selection is too big to do in memory
	if (end_addr - start_addr > (16*1024*1024) && GetDocument()->DataFileSlotFree())
	{
        int idx = -1;                       // Index into docs data_file_ array (or -1 if no slots avail.)

		// Create a file to store the resultant data
		// (Temp file used by the document until it is closed or written to disk.)
		char temp_dir[_MAX_PATH];
		char temp_file[_MAX_PATH];
		::GetTempPath(sizeof(temp_dir), temp_dir);
		::GetTempFileName(temp_dir, _T("_HE"), 0, temp_file);

		// Get data buffer
		size_t len, buflen = size_t(min(4096, end_addr - start_addr));
		try
		{
            buf = new unsigned char[buflen];
        }
        catch(std::bad_alloc)
        {
            ::HMessageBox("Insufficient memory to perform conversion");
            theApp.mac_error_ = 10;
            goto func_return;
        }
        mm->m_wndStatusBar.EnablePaneProgressBar(0, 100);  // turn on progress display
        MSG msg;                                           // use to check for abort key press

		try
		{
		    CFile64 ff(temp_file, CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive|CFile::typeBinary);

			for (FILE_ADDRESS curr = start_addr; curr < end_addr; curr += len)
			{
				// Get the next buffer full from the document
				len = size_t(min(buflen, end_addr - curr));
				if (op != unary_at)      // We don't need to get old value if assigning
				    VERIFY(GetDocument()->GetData(buf, len, curr) == len);

				ProcConversion(buf, len, op);

				ff.Write(buf, len);

				// Do any redrawing, but nothing else
				while (::PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_NOREMOVE))
				{
					if (::GetMessage(&msg, NULL, WM_PAINT, WM_PAINT))
					{
						::TranslateMessage(&msg);
						::DispatchMessage(&msg);
					}
				}

                // Check if a key has been pressed
                if (::PeekMessage(&msg, NULL, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE))
                {
                    // Windows does not like to miss key down events (need to match key up events)
                    ::TranslateMessage(&msg);
                    ::DispatchMessage(&msg);

                    // Remove any characters resulting from keypresses (so they are not inserted into the active file)
                    while (::PeekMessage(&msg, NULL, WM_CHAR, WM_CHAR, PM_REMOVE))
                        ;

					CString mess;
					mess.Format("Abort %s?", desc);
                    if (msg.wParam != 'y' &&  // avoid accidental multiple presses of Y key from aborting
                        ::HMessageBox(mess, MB_YESNO) == IDYES)
                    {
						ff.Close();
						remove(temp_file);
						theApp.mac_error_ = 10;
						goto func_return;
                    }
                }

				mm->m_wndStatusBar.SetPaneProgress(0, long(((curr - start_addr)*100)/(end_addr - start_addr)));
			}
		}
		catch (CFileException *pfe)
		{
			::HMessageBox(::FileErrorMessage(pfe, CFile::modeWrite));
			pfe->Delete();

			remove(temp_file);
			theApp.mac_error_ = 10;
			goto func_return;
		}

		// Delete the unprocessed block that is to be replaced
		// Note: this must be done before AddDataFile otherwise Change() (via regenerate()) will delete the temp file.
        GetDocument()->Change(mod_delforw, start_addr, end_addr - start_addr, NULL, 0, this);

		// Add the temp file to the document
		idx = GetDocument()->AddDataFile(temp_file);
		ASSERT(idx != -1);
        GetDocument()->Change(mod_insert_file, start_addr, end_addr - start_addr, NULL, idx, this, TRUE);
	}
	else
	{
		// Allocate memory block and process it

		// Error if ran out of temp file handles and file is too big
		if (end_addr - start_addr > UINT_MAX)  // why is there no SIZE_T_MAX?
		{
			::HMessageBox("HexEdit is out of temporary files and \n"
                          "cannot convert such a large selection. \n"
                          "Please save the file to deallocate \n"
						  "temporary file handles and try again.\n",
						  MB_OK);
            theApp.mac_error_ = 10;
			return;
		}

		// Warn if we might cause memory exhaustion
        if (end_addr - start_addr > 128*1024*1024)  // 128 Mb file may be too big
        {
			if (::HMessageBox("WARNING: HexEdit is out of temporary file \n"
                              "handles and converting such a large selection \n"
                              "may cause memory exhaustion.  Please click NO \n"
                              "and save the file to free handles. \n"
							  "Or, click YES to continue.\n\n"
                              "Do you want to continue?", MB_YESNO) != IDYES)
			{
                theApp.mac_error_ = 5;
                return;
			}
        }

        CWaitCursor wait;                           // Turn on wait cursor (hourglass)

        try
        {
            buf = new unsigned char[size_t(end_addr - start_addr)];
        }
        catch(std::bad_alloc)
        {
            ::HMessageBox("Insufficient memory to perform conversion");
            theApp.mac_error_ = 10;
            goto func_return;
        }
		size_t got = GetDocument()->GetData(buf, size_t(end_addr - start_addr), start_addr);
		ASSERT(got == size_t(end_addr - start_addr));

		ProcConversion(buf, got, op);

        GetDocument()->Change(mod_replace, start_addr, got, (unsigned char *)buf, 0, this);
	}

    DisplayCaret();
    theApp.SaveToMacro(km_convert, op);

func_return:
	mm->m_wndStatusBar.EnablePaneProgressBar(0, -1);  // disable progress bar

	if (buf != NULL)
		delete[] buf;
}

// Performs particular conversions on a memory buffer
void CHexEditView::ProcConversion(unsigned char *buf, size_t count, convert_type op)
{
    // Operate on all the selected values
    for (size_t ii = 0; ii < count; ++ii)
    {
        switch (op)
        {
        case CONVERT_ASC2EBC:
			if (buf[ii] < 128)
				buf[ii] = a2e_tab[buf[ii]];
			else
				buf[ii] = '\0';
            break;
        case CONVERT_EBC2ASC:
            buf[ii] = e2a_tab[buf[ii]];
            break;
        case CONVERT_ANSI2IBM:
			if (buf[ii] > 128)
				buf[ii] = a2i_tab[buf[ii]&0x7F];
            break;
        case CONVERT_IBM2ANSI:
			if (buf[ii] > 128)
				buf[ii] = i2a_tab[buf[ii]&0x7F];
            break;
		case CONVERT_UPPER:
			if (EbcdicMode())
			{
			    if (buf[ii] >= 0x81 && buf[ii] <= 0x89 ||
				    buf[ii] >= 0x91 && buf[ii] <= 0x99 ||
				    buf[ii] >= 0xA2 && buf[ii] <= 0xA9 )
				{
                    buf[ii] += 0x40;
				}
			}
			else if (ANSIMode() || buf[ii] < 128)
				buf[ii] = toupper(buf[ii]);
            break;
		case CONVERT_LOWER:
			if (EbcdicMode())
			{
			    if (buf[ii] >= 0xC1 && buf[ii] <= 0xC9 ||
				    buf[ii] >= 0xD1 && buf[ii] <= 0xD9 ||
				    buf[ii] >= 0xE2 && buf[ii] <= 0xE9 )
				{
                buf[ii] -= 0x40;
				}
			}
			else if (ANSIMode() || buf[ii] < 128)
				buf[ii] = tolower(buf[ii]);
            break;
        default:
            ASSERT(0);
        }
    }
}

// Make sure there is a selection before doing conversion
void CHexEditView::OnUpdateConvert(CCmdUI* pCmdUI) 
{
    FILE_ADDRESS start_addr, end_addr;              // Current selection
    GetSelAddr(start_addr, end_addr);

    if (pCmdUI->m_pSubMenu != NULL)
    {
        // This happens when popup menu itself is drawn
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION | (start_addr < end_addr ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
    }
    else
        pCmdUI->Enable(start_addr < end_addr);
}

void CHexEditView::OnEncrypt() 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
    if (check_ro("encrypt"))
        return;

    // Get current address or selection
    FILE_ADDRESS start_addr, end_addr;          // Start and end of selection
    GetSelAddr(start_addr, end_addr);

    // Make sure there is a selection
    if (start_addr >= end_addr)
    {
        // No selection, presumably in macro playback
        ASSERT(aa->playing_);
        ::HMessageBox("There is no selection to encrypt");
        aa->mac_error_ = 10;
        return;
    }

    if (aa->password_.IsEmpty())
    {
        CPassword dlg;
        dlg.m_password = aa->password_;
        if (dlg.DoModal() == IDOK)
        {
            aa->password_ = dlg.m_password;
            // Create encryption key with new password
            if (aa->algorithm_ > 0)
            {
                ASSERT(aa->algorithm_ - 1 < (int)aa->crypto_.GetNum());
                aa->crypto_.SetPassword(aa->algorithm_-1, aa->password_);
            }
        }
        else
            return;
    }
    else if (aa->algorithm_ > 0 && aa->crypto_.NeedsPassword(aa->algorithm_-1))
    {
        // We have a password but somehow it has not been set for this alg
        ASSERT(aa->algorithm_ - 1 < (int)aa->crypto_.GetNum());
        aa->crypto_.SetPassword(aa->algorithm_-1, aa->password_);
    }

    if (aa->algorithm_ > 0)
    {
#ifdef ENCRYPT_MULT_BLOCK  // We don't need to restrict CSP block encryption to multiple of block size
        int blen = aa->crypto_.GetBlockLength(aa->algorithm_-1);
        if (blen == -1)
        {
            ::HMessageBox("Encryption block length error");
            aa->mac_error_ = 10;
            return;
        }

        ASSERT(blen%8 == 0); // Should be multiple of number of bits/byte
        if (blen == 0)
            blen = 1;       // Stream cipher - don't restrict selection length
        else
            blen = blen/8;  // Convert bits to bytes

        // Make sure selection is right size
        if ((end_addr-start_addr)%blen != 0)
        {
            // Presumably in macro playback
            ASSERT(aa->playing_);
            ::HMessageBox("The selection is not a multiple of the encryption block length");
            aa->mac_error_ = 10;
            return;
        }
#endif

        size_t len = size_t(end_addr - start_addr); // Length of selection
        size_t buflen = aa->crypto_.needed(aa->algorithm_-1, len);

        if (display_.overtype && len < buflen)
        {
            if (::HMessageBox("Encrypting with this algorithm will increase the\r"
                              "the length of the current (unencrypted) selection.\r"
                              "Do you want to turn off overtype mode?",
                              MB_OKCANCEL) == IDCANCEL)
            {
                aa->mac_error_ = 10;
                return;
            }
			else if (!do_insert())
				return;
        }

        // Get memory for selection and read it from the document
        unsigned char *buf = new unsigned char[buflen];
        size_t got = GetDocument()->GetData(buf, len, start_addr);
        ASSERT(got == len);

        if (aa->crypto_.encrypt(aa->algorithm_-1, buf, len, buflen) != buflen)
        {
            ASSERT(0);
            ::HMessageBox("Encryption error");
            aa->mac_error_ = 10;
            delete[] buf;
            return;
        }

        if (len != buflen)
        {
            ASSERT(buflen > len);
#ifdef ENCRYPT_MULT_BLOCK
            ASSERT((buflen - len)%blen == 0);
#endif

            // Since the encrypted text is bigger we must delete then insert
            GetDocument()->Change(mod_delforw, start_addr, len, NULL, 0, this);
            GetDocument()->Change(mod_insert, start_addr, buflen, buf, 0, this, TRUE);
            ASSERT(undo_.back().utype == undo_change);
            undo_.back().previous_too = true;    // Merge changes (at least in this view)

            // Select encrypted data and save undo of selection
            SetSel(addr2pos(start_addr), addr2pos(start_addr+buflen));
            undo_.push_back(view_undo(undo_move, TRUE));  // Save selection start in undo array
            undo_.back().address = start_addr;
            undo_.push_back(view_undo(undo_sel, TRUE));   // Save selection end in undo array
            undo_.back().address = end_addr;

            // Inform the user in case they don't notice the selection has been increased
            CString mess;
            mess.Format("Encryption increased the selection length by %ld bytes", long(buflen-len));
            ((CMainFrame *)AfxGetMainWnd())->StatusBarText(mess);
        }
        else
            GetDocument()->Change(mod_replace, start_addr, len, buf, 0, this);
        delete[] buf;
    }
    else
    {
        // Note that CryptoAPI algs (above) the key is set when password
        // or alg is changed (more efficient) but for internal we set it here
        // in case other internal use (security) has changed the current key
        ::set_key(aa->password_, aa->password_.GetLength());

        // Make sure selection is right size
        if ((end_addr-start_addr)%8 != 0)
        {
            // Presumably in macro playback
            ASSERT(aa->playing_);
            ::HMessageBox("The selection is not a multiple of 8 (encryption block length)");
            aa->mac_error_ = 10;
            return;
        }

        size_t len = size_t(end_addr - start_addr); // Length of selection

        // Get memory for selection and read it from the document
        unsigned char *buf = new unsigned char[len];
        size_t got = GetDocument()->GetData(buf, len, start_addr);
        ASSERT(got == len);

        ::encrypt(buf, len);

        GetDocument()->Change(mod_replace, start_addr, len, buf, 0, this);
        delete[] buf;
    }
    DisplayCaret();
    aa->SaveToMacro(km_encrypt, 1);  // 1 == encrypt (2 == decrypt)
}

void CHexEditView::OnDecrypt() 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
    if (check_ro("decrypt"))
        return;

    // Get current address or selection
    FILE_ADDRESS start_addr, end_addr;          // Start and end of selection
    GetSelAddr(start_addr, end_addr);

    // Make sure there is a selection
    if (start_addr >= end_addr)
    {
        // No selection, presumably in macro playback
        ASSERT(aa->playing_);
        ::HMessageBox("There is no selection to decrypt");
        aa->mac_error_ = 10;
        return;
    }

    if (aa->password_.IsEmpty())
    {
        CPassword dlg;
        dlg.m_password = aa->password_;
        if (dlg.DoModal() == IDOK)
        {
            aa->password_ = dlg.m_password;
            // Create encryption key with new password
            if (aa->algorithm_ > 0)
            {
                ASSERT(aa->algorithm_ - 1 < (int)aa->crypto_.GetNum());
                aa->crypto_.SetPassword(aa->algorithm_-1, aa->password_);
            }
        }
        else
            return;
    }
    else if (aa->algorithm_ > 0 && aa->crypto_.NeedsPassword(aa->algorithm_-1))
    {
        // We have a password but somehow it has not been set for this alg
        ASSERT(aa->algorithm_ - 1 < (int)aa->crypto_.GetNum());
        aa->crypto_.SetPassword(aa->algorithm_-1, aa->password_);
    }

    if (aa->algorithm_ > 0)
    {
        int blen = aa->crypto_.GetBlockLength(aa->algorithm_-1);
        if (blen == -1)
        {
            ::HMessageBox("Encryption block length error");
            aa->mac_error_ = 10;
            return;
        }

        ASSERT(blen%8 == 0); // Should be multiple of number of bits/byte
        if (blen == 0)
            blen = 1;       // Stream cipher - don't restrict selection length
        else
            blen = blen/8;  // Convert bits to bytes

        // Make sure selection is right size
        if ((end_addr-start_addr)%blen != 0)
        {
            // Presumably in macro playback
            ASSERT(aa->playing_);
            ::HMessageBox("The selection is not a multiple of encryption block length");
            aa->mac_error_ = 10;
            return;
        }

        size_t len = size_t(end_addr - start_addr); // Length of selection
        size_t newlen;          // Size of decrypted text (may be less than encrypted)

        // Get memory for selection and read it from the document
        unsigned char *buf = new unsigned char[len];
        size_t got = GetDocument()->GetData(buf, len, start_addr);
        ASSERT(got == len);

        newlen = aa->crypto_.decrypt(aa->algorithm_-1, buf, len);
        if (newlen == size_t(-1))
        {
            ::HMessageBox(CString("Could not decrypt using:\n") + aa->crypto_.GetName(aa->algorithm_-1));
            aa->mac_error_ = 10;
            delete[] buf;
            return;
        }

        if (len != newlen)
        {
            ASSERT(newlen < len);
#ifdef ENCRYPT_MULT_BLOCK
            ASSERT((len - newlen)%blen == 0);
#endif

            if (display_.overtype)
            {
                if (::HMessageBox("Decrypting with this algorithm will decrease the\r"
                                  "the length of the current (encrypted) selection.\r"
                                  "Do you want to turn off overtype mode?",
                                  MB_OKCANCEL) == IDCANCEL)
                {
                    aa->mac_error_ = 10;
                    delete[] buf;
                    return;
                }
				else if (!do_insert())
					return;
            }

            // Since the decrypted text is smaller we must delete then insert
            GetDocument()->Change(mod_delforw, start_addr, len, NULL, 0, this);
            GetDocument()->Change(mod_insert, start_addr, newlen, buf, 0, this, TRUE);
            ASSERT(undo_.back().utype == undo_change);
            undo_.back().previous_too = true;    // Merge changes (at least in this view)

            // Select encrypted data and save undo of selection
            SetSel(addr2pos(start_addr), addr2pos(start_addr+newlen));
            undo_.push_back(view_undo(undo_move, TRUE));  // Save selection start in undo array
            undo_.back().address = start_addr;
            undo_.push_back(view_undo(undo_sel, TRUE));   // Save selection end in undo array
            undo_.back().address = end_addr;

            // Inform the user in case they don't notice the selection has been decreased
            CString mess;
            mess.Format("Decryption decreased the selection length by %ld bytes", long(len-newlen));
            ((CMainFrame *)AfxGetMainWnd())->StatusBarText(mess);
        }
        else
            GetDocument()->Change(mod_replace, start_addr, len, buf, 0, this);
        delete[] buf;
    }
    else
    {
        // Note that CryptoAPI algs (above) the key is set when password
        // or alg is changed (more efficient) but for internal we set it here
        // in case other internal use (security) has changed the current key
        ::set_key(aa->password_, aa->password_.GetLength());

        // Make sure selection is right size
        if ((end_addr-start_addr)%8 != 0)
        {
            // Presumably in macro playback
            ASSERT(aa->playing_);
            ::HMessageBox("The selection is not a multiple of 8 (encryption block length)");
            aa->mac_error_ = 10;
            return;
        }

        size_t len = size_t(end_addr - start_addr); // Length of selection

        // Get memory for selection and read it from the document
        unsigned char *buf = new unsigned char[len];
        size_t got = GetDocument()->GetData(buf, len, start_addr);
        ASSERT(got == len);

        ::decrypt(buf, len);

        GetDocument()->Change(mod_replace, start_addr, len, buf, 0, this);
        delete[] buf;
    }
    DisplayCaret();
    aa->SaveToMacro(km_encrypt, 2);  // 2 == decrypt (1 == encrypt)
}

void CHexEditView::OnUpdateEncrypt(CCmdUI* pCmdUI) 
{
    CHexEditApp *aa = dynamic_cast<CHexEditApp *>(AfxGetApp());
    FILE_ADDRESS start_addr, end_addr;              // Current selection
    GetSelAddr(start_addr, end_addr);
    if (start_addr == end_addr)
        pCmdUI->Enable(FALSE);
    else if (aa->algorithm_ == 0)
    {
        // Internal algorithm must encrypt mult. of 8 bytes (Blow Fish has 64 bit block length)
        pCmdUI->Enable((end_addr-start_addr)%8 == 0);
    }
#ifdef ENCRYPT_MULT_BLOCK  // We don't need to restrict CSP block encryption to multiple of block size
    else
    {
        int blen = aa->crypto_.GetBlockLength(aa->algorithm_-1);
        if (blen == -1)
        {
            pCmdUI->Enable(FALSE);
            return;
        }

        ASSERT(blen%8 == 0); // Should be multiple of number of bits/byte
        ASSERT(blen%8 == 0);
        if (blen == 0)
            blen = 1;       // Stream cipher - don't restrict selection length
        else
            blen = blen/8;  // Convert bits to bytes

        // Make sure selection length is multiple of block length
        pCmdUI->Enable((end_addr-start_addr)%blen == 0);
    }
#endif
}

void CHexEditView::OnCompress() 
{
    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();
    unsigned char *in_data = NULL;
    unsigned char *out_data = NULL;

	// Check if file is read-only and ask user if they want to turn this off
	if (check_ro("compress"))
		return;

	// Check if in OVR mode and ask the user if they want to turn this off
    if (display_.overtype)
    {
        if (::HMessageBox("Compression will change the selection length\r"
                            "which is not allowed in overtype mode.\r"
                            "Do you want to turn off overtype mode?",
                            MB_OKCANCEL) == IDCANCEL)
        {
            theApp.mac_error_ = 10;
            return;
        }
		else if (!do_insert())
			return;
    }

    // Get current address or selection
    FILE_ADDRESS start_addr, end_addr;          // Start and end of selection
    GetSelAddr(start_addr, end_addr);

    // Make sure there is a selection
    if (start_addr >= end_addr)
    {
        // No selection, presumably in macro playback
        ASSERT(theApp.playing_);
        ::HMessageBox("There is no selection to compress");
        theApp.mac_error_ = 10;
        return;
    }

	// Initialise structure used to communicate with the zlib routines
	z_stream zs;
	zs.zalloc = (alloc_func)0;
	zs.zfree = (free_func)0;
	zs.opaque = (voidpf)0;
	VERIFY(deflateInit(&zs, Z_DEFAULT_COMPRESSION) == Z_OK);

	// Test if selection is too big to do in memory
	if (end_addr - start_addr > (16*1024*1024) && GetDocument()->DataFileSlotFree())
	{
        int idx = -1;                       // Index into docs data_file_ array (or -1 if no slots avail.)

		// Create a "temp" file to store the compressed data
		// (This file stores the compressed data until the document is closed or written to disk.)
		char temp_dir[_MAX_PATH];
		char temp_file[_MAX_PATH];
		::GetTempPath(sizeof(temp_dir), temp_dir);
		::GetTempFileName(temp_dir, _T("_HE"), 0, temp_file);

		// Get input and output buffers
		size_t len, inbuflen = size_t(min(32768, end_addr - start_addr)), outbuflen = max(inbuflen/2, 256);
		int sync_every = (1024*1024)/inbuflen;    // No of input blocks per sync (currently every 1MB)
		FILE_ADDRESS total_out = 0;
		int err;
		try
		{
            in_data = new unsigned char[inbuflen];
            out_data = new unsigned char[outbuflen];
        }
        catch(std::bad_alloc)
        {
            ::HMessageBox("Insufficient memory to perform compression");
            theApp.mac_error_ = 10;
            goto func_return;
        }
        mm->m_wndStatusBar.EnablePaneProgressBar(0, 100);  // turn on progress display
        MSG msg;                                           // use to check for abort key press

		try
		{
		    CFile64 ff(temp_file, CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive|CFile::typeBinary);

			for (FILE_ADDRESS curr = start_addr; curr < end_addr; curr += len)
			{
				int flush;
				if ((curr - start_addr)/inbuflen % sync_every == sync_every - 1)  // time to sync?
					flush = Z_FULL_FLUSH;
				else
					flush = Z_NO_FLUSH;

				// Get the next buffer full from the document
				len = size_t(min(inbuflen, end_addr - curr));
				VERIFY(GetDocument()->GetData(in_data, len, curr) == len);

				// Compress this buffer
				zs.next_in = in_data;
				zs.avail_in = len;
				do
				{
					zs.next_out = out_data;
					zs.avail_out = outbuflen;
					err = deflate(&zs, flush);
					ASSERT(err == Z_OK);

					// Write to "temp" file
					ff.Write(out_data, outbuflen - zs.avail_out);

					total_out += outbuflen - zs.avail_out;  // we need to keep track of this ourselves since zs.total_out can overflow
				} while (zs.avail_out == 0);
				ASSERT(zs.avail_in == 0);

				// Do any redrawing, but nothing else
				while (::PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_NOREMOVE))
				{
					if (::GetMessage(&msg, NULL, WM_PAINT, WM_PAINT))
					{
						::TranslateMessage(&msg);
						::DispatchMessage(&msg);
					}
				}

                // Check if a key has been pressed
                if (::PeekMessage(&msg, NULL, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE))
                {
                    // Windows does not like to miss key down events (need to match key up events)
                    ::TranslateMessage(&msg);
                    ::DispatchMessage(&msg);

                    // Remove any characters resulting from keypresses (so they are not inserted into the active file)
                    while (::PeekMessage(&msg, NULL, WM_CHAR, WM_CHAR, PM_REMOVE))
                        ;

                    if (msg.wParam != 'y' &&  // avoid accidental multiple presses of Y key from aborting
                        ::HMessageBox("Abort compression?", MB_YESNO) == IDYES)
                    {
						ff.Close();
						remove(temp_file);
						theApp.mac_error_ = 10;
						goto func_return;
                    }
                }

				mm->m_wndStatusBar.SetPaneProgress(0, long(((curr - start_addr)*100)/(end_addr - start_addr)));
			}

			// Process any residual data
			do
			{
				zs.next_out = out_data;
				zs.avail_out = outbuflen;
			    err = deflate(&zs, Z_FINISH);
				ASSERT(err == Z_OK || err == Z_STREAM_END);

				ff.Write(out_data, outbuflen - zs.avail_out);
				total_out += outbuflen - zs.avail_out;
			} while (err == Z_OK);
			ASSERT(err == Z_STREAM_END);
		}
		catch (CFileException *pfe)
		{
			::HMessageBox(::FileErrorMessage(pfe, CFile::modeWrite));
			pfe->Delete();

			remove(temp_file);
			theApp.mac_error_ = 10;
			goto func_return;
		}

		// Delete the uncompressed block that is to be replaced
		// Note: this must be done before AddDataFile otherwise Change() (via regenerate()) will delete the temp file.
        GetDocument()->Change(mod_delforw, start_addr, end_addr - start_addr, NULL, 0, this);

		// Add the temp file to the document
		idx = GetDocument()->AddDataFile(temp_file);
		ASSERT(idx != -1);
        GetDocument()->Change(mod_insert_file, start_addr, total_out, NULL, idx, this, TRUE);

        // Select compressed data
        SetSel(addr2pos(start_addr), addr2pos(start_addr + total_out));

        // Inform the user about the amount of compression
        CString mess;
        mess.Format("Compressed %d%%", int((1.0 - double(total_out)/(end_addr - start_addr))*100.0 + 0.5));
        mm->StatusBarText(mess);
	}
	else
	{
		// Allocate memory block and compress into it

		// Error if ran out of temp file handles and file is too big
		if (end_addr - start_addr > UINT_MAX)  // why is there no SIZE_T_MAX?
		{
			::HMessageBox("HexEdit is out of temporary files and \n"
                          "cannot compress such a large selection. \n"
                          "Please save the file to free \n"
						  "temporary file handles and try again.\n",
						  MB_OK);
            theApp.mac_error_ = 10;
			return;
		}

		// Warn if we might cause memory exhaustion
        if (end_addr - start_addr > 128*1024*1024)  // 128 Mb file may be too big
        {
			if (::HMessageBox("WARNING: HexEdit is out of temporary file \n"
                              "handles and compressing such a large selection \n"
                              "may cause memory exhaustion.  Please click NO \n"
                              "and save the file to free handles. \n"
							  "Or, click YES to continue.\n\n"
                              "Do you want to continue?", MB_YESNO) != IDYES)
			{
                theApp.mac_error_ = 5;
                return;
			}
        }

        CWaitCursor wait;                           // Turn on wait cursor (hourglass)

        try
        {
            // Get buffer for data, get the data, and get buffer for compressed data
            in_data = new unsigned char[size_t(end_addr - start_addr)];
            size_t got = GetDocument()->GetData(in_data, size_t(end_addr - start_addr), start_addr);
			ASSERT(got == size_t(end_addr - start_addr));

			// Remove uncompressed data from the document
            GetDocument()->Change(mod_delforw, start_addr, got, NULL, 0, this);

		    size_t outbuflen = max(got/2, 256);
            out_data = new unsigned char[outbuflen];
			FILE_ADDRESS curr = start_addr;

		    int err;    // return value from inflate (normally Z_OK or Z_STREAM_END)

			zs.next_in = in_data;
			zs.avail_in = got;
			do
			{
				zs.next_out = out_data;
				zs.avail_out = outbuflen;
				err = deflate(&zs, Z_FINISH);
				ASSERT(err == Z_OK || err == Z_STREAM_END);

				// Replace the selection with the compressed data
				if (outbuflen - zs.avail_out > 0)
				    GetDocument()->Change(mod_insert, curr, outbuflen - zs.avail_out, out_data, 0, this, TRUE);

				curr += outbuflen - zs.avail_out;
			} while (err == Z_OK);
			ASSERT(err == Z_STREAM_END);

            // Select compressed block
            SetSel(addr2pos(start_addr), addr2pos(start_addr+zs.total_out));

            // Inform the user about the amount of compression
            CString mess;
            mess.Format("Compressed %d%%", int((1.0 - double(zs.total_out)/got)*100.0 + 0.5));
            mm->StatusBarText(mess);
        }
        catch(std::bad_alloc)
        {
            ::HMessageBox("Insufficient memory to perform compression");
            theApp.mac_error_ = 10;

			// Undo changes already (but make sure we don't record undo in macros)
			OnEditUndo();
            if (theApp.recording_ && theApp.mac_.size() > 0 && (theApp.mac_.back()).ktype == km_undo)
                theApp.mac_.pop_back();

            goto func_return;
        }
	}
	VERIFY(deflateEnd(&zs) == Z_OK);

    DisplayCaret();
    theApp.SaveToMacro(km_compress, 1);  // 1 = compress (2 = decompress)

func_return:
	mm->m_wndStatusBar.EnablePaneProgressBar(0, -1);  // disable progress bar

	if (in_data != NULL)
		delete[] in_data;
	if (out_data != NULL)
		delete[] out_data;
}

void CHexEditView::OnDecompress() 
{
    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();
    unsigned char *in_data = NULL;
    unsigned char *out_data = NULL;

	// Check if file is read-only and ask user if they want to turn this off
	if (check_ro("decompress"))
		return;

	// Check if in OVR mode and ask the user if they want to turn this off
    if (display_.overtype)
    {
        if (::HMessageBox("Decompression will change the selection length\r"
                            "which is not allowed in overtype mode.\r"
                            "Do you want to turn off overtype mode?",
                            MB_OKCANCEL) == IDCANCEL)
        {
            theApp.mac_error_ = 10;
            return;
        }
		else if (!do_insert())
			return;
    }

    // Get current address or selection
    FILE_ADDRESS start_addr, end_addr;          // Start and end of selection
    GetSelAddr(start_addr, end_addr);

    // Make sure there is a selection
    if (start_addr >= end_addr)
    {
        // No selection, presumably in macro playback
        ASSERT(theApp.playing_);
        ::HMessageBox("There is no selection to decompress");
        theApp.mac_error_ = 10;
        return;
    }

	// Initialise structure used to communicate with the zlib routines
	z_stream zs;
	zs.zalloc = (alloc_func)0;
	zs.zfree = (free_func)0;
	zs.opaque = (voidpf)0;
	zs.next_in = (Bytef*)0;
	zs.avail_in = 0;
	VERIFY(inflateInit(&zs) == Z_OK);

	// Test if selection is too big to do in memory
	if (end_addr - start_addr > (16*1024*1024) && GetDocument()->DataFileSlotFree())
	{
        int idx = -1;                       // Index into docs data_file_ array (or -1 if no slots avail.)

		// Create a "temp" file to store the compressed data
		// (This file stores the compressed data until the document is closed or written to disk.)
		char temp_dir[_MAX_PATH];
        char temp_file[_MAX_PATH];
		::GetTempPath(sizeof(temp_dir), temp_dir);
		::GetTempFileName(temp_dir, _T("_HE"), 0, temp_file);

		// Get input and output buffers
		size_t len, outbuflen = size_t(min(32768, end_addr - start_addr)), inbuflen = max(outbuflen/4, 256);
		FILE_ADDRESS total_out = 0;
		int err = Z_OK;
		try
		{
            in_data = new unsigned char[inbuflen];
            out_data = new unsigned char[outbuflen];
        }
        catch(std::bad_alloc)
        {
            ::HMessageBox("Insufficient memory to perform decompression");
            theApp.mac_error_ = 10;
            goto func_return;
        }
        mm->m_wndStatusBar.EnablePaneProgressBar(0, 100);  // turn on progress display
        MSG msg;                                           // use to check for abort key press

		try
		{
		    CFile64 ff(temp_file, CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive|CFile::typeBinary);

			// For each chunk of the currently selected block
			for (FILE_ADDRESS curr = start_addr; curr < end_addr; curr += len)
			{
				// Get the next buffer full from the document
				len = size_t(min(inbuflen, end_addr - curr));
				VERIFY(GetDocument()->GetData(in_data, len, curr) == len);

				zs.next_in = in_data;
				zs.avail_in = len;
				do
				{
					// Prepare output buffer
					zs.next_out = out_data;
					zs.avail_out = outbuflen;
					if (err == Z_DATA_ERROR)
					{
						// We must still be trying to sync from previously encountered data error
						err = inflateSync(&zs);
					}
					else
					{
						err = inflate(&zs, Z_NO_FLUSH);
						ASSERT(err == Z_OK || err == Z_STREAM_END || err == Z_DATA_ERROR || err == Z_BUF_ERROR);
						// Note Z_BUF_ERROR can occur when zs.avail_out == 0 (and zs.avail_in after previous call was zero).
						// This is not a fatal error and according to the docs we should continue looping while zs.avail_in == 0.

						if (err == Z_DATA_ERROR)
						{
							theApp.mac_error_ = 5;
							if (::HMessageBox("The compression data is corrupted. \n"
									    	  "HexEdit save the data decompressed \n"
										      "so far and attempt to recover from \n"
									          "this error but some data will be lost. \n\n"
										      "Do you want to continue?", MB_YESNO) != IDYES)
							{
								ff.Close();
								remove(temp_file);
								theApp.mac_error_ = 10;
								goto func_return;
							}
							// Continue reading the data checking for sync point (see inflateSync call above)
						}

						ff.Write(out_data, outbuflen - zs.avail_out);  // write all that we got
						total_out += outbuflen - zs.avail_out;  // we need to keep track of this ourselves since zs.total_out can overflow
					}
				} while (zs.avail_out == 0 || zs.avail_in != 0);  // if output buffer is full there may be more

				// Do any redrawing, but nothing else
				while (::PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_NOREMOVE))
				{
					if (::GetMessage(&msg, NULL, WM_PAINT, WM_PAINT))
					{
						::TranslateMessage(&msg);
						::DispatchMessage(&msg);
					}
				}

                // Check if a key has been pressed
                if (::PeekMessage(&msg, NULL, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE))
                {
                    // Windows does not like to miss key down events (need to match key up events)
                    ::TranslateMessage(&msg);
                    ::DispatchMessage(&msg);

                    // Remove any characters resulting from keypresses (so they are not inserted into the active file)
                    while (::PeekMessage(&msg, NULL, WM_CHAR, WM_CHAR, PM_REMOVE))
                        ;

					if (msg.wParam != 'y' &&  // avoid accidental multiple presses of Y key from aborting
						::HMessageBox("Abort decompression?", MB_YESNO) == IDYES)
                    {
						ff.Close();
                        remove(temp_file);
						theApp.mac_error_ = 10;
						goto func_return;
                    }
                }

				mm->m_wndStatusBar.SetPaneProgress(0, long(((curr - start_addr)*100)/(end_addr - start_addr)));
			}

			ASSERT(err == Z_STREAM_END || err == Z_DATA_ERROR);
			ASSERT(ff.GetLength() == total_out);
			if (err == Z_DATA_ERROR)
			{
				::HMessageBox("HexEdit did not recover from \n"
								"the compression data error. ");
			}
		}
		catch (CFileException *pfe)
		{
			::HMessageBox(::FileErrorMessage(pfe, CFile::modeWrite));
			pfe->Delete();

            remove(temp_file);
			theApp.mac_error_ = 10;
			goto func_return;
		}

		if (total_out == 0)
		{
			// This could happen if there were data errors
            remove(temp_file);
			theApp.mac_error_ = 5;
			goto func_return;
		}

		// Delete the input (compressed) block that is to be replaced
		// Note: this must be done before AddDataFile otherwise Change() (via regenerate()) will delete the temp file.
        GetDocument()->Change(mod_delforw, start_addr, end_addr - start_addr, NULL, 0, this);

		// Add the temp file to the document
		idx = GetDocument()->AddDataFile(temp_file);
		ASSERT(idx != -1);
        GetDocument()->Change(mod_insert_file, start_addr, total_out, NULL, idx, this, TRUE);

        // Select uncompressed block
        SetSel(addr2pos(start_addr), addr2pos(start_addr + total_out));

        // Inform the user about the amount of compression
        CString mess;
        mess.Format("Expanded by %d%%", int((double(total_out)/(end_addr - start_addr) - 1.0)*100.0 + 0.5));
        mm->StatusBarText(mess);
	}
	else
	{
		// Allocate memory block and decompress into it

		// Error if ran out of temp file handles and file is too big
		if (end_addr - start_addr > UINT_MAX)  // why is there no SIZE_T_MAX?
		{
			::HMessageBox("HexEdit is out of temporary files and \n"
                          "cannot decompress such a large selection. \n"
                          "Please save the file to free \n"
						  "temporary file handles and try again.\n",
						  MB_OK);
            theApp.mac_error_ = 10;
			return;
		}

		// Warn if we might cause memory exhaustion
        if (end_addr - start_addr > 128*1024*1024)  // 128 Mb file may be too big
        {
			if (::HMessageBox("WARNING: HexEdit is out of temporary file \n"
                              "handles and decompressing such a large selection \n"
                              "may cause memory exhaustion.  Please click NO \n"
                              "and save the file to free handles. \n"
							  "Or, click YES to continue.\n\n"
                              "Do you want to continue?", MB_YESNO) != IDYES)
			{
                theApp.mac_error_ = 5;
                return;
			}
        }

        CWaitCursor wait;                           // Turn on wait cursor (hourglass)

        try
        {
            // Get buffer for input data, get the data, and get buffer for output (decompressed) data
            in_data = new unsigned char[size_t(end_addr - start_addr)];
            size_t got = GetDocument()->GetData(in_data, size_t(end_addr - start_addr), start_addr);
			ASSERT(got == size_t(end_addr - start_addr));

			// Remove input (compressed) block from the document
            GetDocument()->Change(mod_delforw, start_addr, got, NULL, 0, this);

		    size_t outbuflen = max(4*got, 256);
            out_data = new unsigned char[outbuflen];
			FILE_ADDRESS curr = start_addr;   // Current output address

		    int err = Z_OK;    // return value from inflate or inflateSync

			zs.next_in = in_data;
			zs.avail_in = got;
			do
			{
				zs.next_out = out_data;
				zs.avail_out = outbuflen;
				if (err == Z_DATA_ERROR)
				{
					// We must still be trying to sync from previously encountered data error
					err = inflateSync(&zs);
				}
				if (err != Z_DATA_ERROR)
				{
					err = inflate(&zs, Z_NO_FLUSH);
					ASSERT(err == Z_OK || err == Z_STREAM_END || err == Z_BUF_ERROR || err == Z_DATA_ERROR);
					if (err == Z_DATA_ERROR)
					{
						if (::HMessageBox("The compression data is corrupted. \n"
											"HexEdit can attempt to recover from \n"
											"this error but some data will be lost. \n\n"
											"Do you want to continue?", MB_YESNO) != IDYES)
						{
							theApp.mac_error_ = 10;

							// Undo changes already (but make sure we don't record undo in macros)
							OnEditUndo();
							if (theApp.recording_ && theApp.mac_.size() > 0 && (theApp.mac_.back()).ktype == km_undo)
								theApp.mac_.pop_back();

							goto func_return;
						}
					}

					// Add the decompressed data block to the file
					if (outbuflen - zs.avail_out > 0)
						GetDocument()->Change(mod_insert, curr, outbuflen - zs.avail_out, out_data, 0, this, TRUE);

					curr += outbuflen - zs.avail_out;
				}
			} while (zs.avail_out == 0 || zs.avail_in != 0);
            ASSERT(err == Z_STREAM_END);

            // Select uncompressed block
            SetSel(addr2pos(start_addr), addr2pos(start_addr+zs.total_out));

            // Inform the user about the amount of compression
            CString mess;
	        mess.Format("Expanded by %d%%", int((double(zs.total_out)/got - 1.0)*100.0 + 0.5));
            mm->StatusBarText(mess);
        }
        catch(std::bad_alloc)
        {
            ::HMessageBox("Insufficient memory to perform decompression");
            theApp.mac_error_ = 10;

			// Undo changes already (but make sure we don't record undo in macros)
			OnEditUndo();
            if (theApp.recording_ && theApp.mac_.size() > 0 && (theApp.mac_.back()).ktype == km_undo)
                theApp.mac_.pop_back();

            goto func_return;
        }
	}
	VERIFY(inflateEnd(&zs) == Z_OK);

    DisplayCaret();
    theApp.SaveToMacro(km_compress, 2);  // 2 = decompress (1 = compress)

func_return:
	mm->m_wndStatusBar.EnablePaneProgressBar(0, -1);  // disable progress bar

	if (in_data != NULL)
		delete[] in_data;
	if (out_data != NULL)
		delete[] out_data;
}

// Enable commands that depend simply on a non-zero selection
void CHexEditView::OnUpdateSelNZ(CCmdUI* pCmdUI) 
{
    FILE_ADDRESS start_addr, end_addr;              // Current selection
    GetSelAddr(start_addr, end_addr);
    pCmdUI->Enable(start_addr < end_addr);
}

// The following functions are used as update handlers
// They enable their corresponding UI handlers if there are
// enough bytes between the current address and the end of file
void CHexEditView::OnUpdateByte(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(GetPos() < GetDocument()->length());
}

void CHexEditView::OnUpdate16bit(CCmdUI* pCmdUI) 
{
    FILE_ADDRESS start_addr, end_addr;              // Current selection
    GetSelAddr(start_addr, end_addr);
    if (start_addr == end_addr)
        // No selection so only enable if enough before EOF (2 bytes)
        pCmdUI->Enable(start_addr + 1 < GetDocument()->length());
    else
        // Else make sure selection is multiple of 2
        pCmdUI->Enable((end_addr - start_addr)%2 == 0);
}

void CHexEditView::OnUpdate32bit(CCmdUI* pCmdUI) 
{
//    pCmdUI->Enable(GetPos() + 3 < GetDocument()->length());
    FILE_ADDRESS start_addr, end_addr;              // Current selection
    GetSelAddr(start_addr, end_addr);
    if (start_addr == end_addr)
        // No selection so only enable if enough before EOF (4 bytes)
        pCmdUI->Enable(start_addr + 3 < GetDocument()->length());
    else
        // Else make sure selection is multiple of 4
        pCmdUI->Enable((end_addr - start_addr)%4 == 0);
}

void CHexEditView::OnUpdate64bit(CCmdUI* pCmdUI) 
{
//    pCmdUI->Enable(GetPos() + 7 < GetDocument()->length());
    FILE_ADDRESS start_addr, end_addr;              // Current selection
    GetSelAddr(start_addr, end_addr);
    if (start_addr == end_addr)
        // No selection so only enable if enough before EOF (8 bytes)
        pCmdUI->Enable(start_addr + 7 < GetDocument()->length());
    else
        // Else make sure selection is multiple of 8
        pCmdUI->Enable((end_addr - start_addr)%8 == 0);
}

// These update handlers are similar to the above but for binary operations
// As binary operations use the current value from the calculator we also need
// to ensure that the current operand size in the calculator is not too big.
void CHexEditView::OnUpdateByteBinary(CCmdUI* pCmdUI) 
{
    if (((CMainFrame *)AfxGetMainWnd())->m_wndCalc.ByteSize() > 1)
    {
        pCmdUI->Enable(FALSE);
        return;
    }
    OnUpdateByte(pCmdUI);
}

void CHexEditView::OnUpdate16bitBinary(CCmdUI* pCmdUI) 
{
    if (((CMainFrame *)AfxGetMainWnd())->m_wndCalc.ByteSize() > 2)
    {
        pCmdUI->Enable(FALSE);
        return;
    }
    OnUpdate16bit(pCmdUI);
}

void CHexEditView::OnUpdate32bitBinary(CCmdUI* pCmdUI) 
{
    if (((CMainFrame *)AfxGetMainWnd())->m_wndCalc.ByteSize() > 4)
    {
        pCmdUI->Enable(FALSE);
        return;
    }
    OnUpdate32bit(pCmdUI);
}

void CHexEditView::OnUpdate64bitBinary(CCmdUI* pCmdUI) 
{
    if (((CMainFrame *)AfxGetMainWnd())->m_wndCalc.ByteSize() > 8)
    {
        ASSERT(0);                     // Max size is 8 at present
        pCmdUI->Enable(FALSE);
        return;
    }
    OnUpdate64bit(pCmdUI);
}

template<class T> void CHexEditView::DoChecksum(checksum_type op, LPCSTR desc)
{
    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();
	T val = 0;
    unsigned char *buf = NULL;

    // Get current address or selection
    FILE_ADDRESS start_addr, end_addr;          // Start and end of selection
    GetSelAddr(start_addr, end_addr);

    if (start_addr >= end_addr)
    {
        // No selection, presumably in macro playback
        ASSERT(theApp.playing_);
        CString mess;
        mess.Format("There is no selection to calculate %s on", desc);
        ::HMessageBox(mess);
        theApp.mac_error_ = 10;
        return;
    }
    if (op >= CHECKSUM_8 && op < 10 && (end_addr - start_addr)%sizeof(T) != 0)
    {
        // Selection is wrong length, presumably in macro playback
        ASSERT(theApp.playing_);
        CString mess;
        mess.Format("The selection must be a multiple of %d for %s", sizeof(T), desc);
        ::HMessageBox(mess);
        theApp.mac_error_ = 10;
        return;
    }
    ASSERT(start_addr < GetDocument()->length());

	// Get a buffer - fairly large for efficiency
	size_t len, buflen = size_t(min(4096, end_addr - start_addr));
	try
	{
        buf = new unsigned char[buflen];
    }
    catch(std::bad_alloc)
    {
        ::HMessageBox("Insufficient memory to perform CRC");
        theApp.mac_error_ = 10;
        return;
    }
	ASSERT(buf != NULL);
    mm->m_wndStatusBar.EnablePaneProgressBar(0, 100);  // turn on progress display
    MSG msg;                                           // use to check for abort key press

	switch (op)
	{
	case CHECKSUM_CRC_CCITT:
		crc_ccitt_init();
		break;
	case CHECKSUM_CRC32:
		crc_32_init();
		break;
	default:
		ASSERT(0);
	}
	for (FILE_ADDRESS curr = start_addr; curr < end_addr; curr += len)
	{
		// Get the next buffer full from the document
		len = size_t(min(buflen, end_addr - curr));
	    VERIFY(GetDocument()->GetData(buf, len, curr) == len);

		switch (op)
		{
		case CHECKSUM_8:
		case CHECKSUM_16:
		case CHECKSUM_32:
		case CHECKSUM_64:
			ASSERT(len % sizeof(T) == 0);
			{
				T *pp, *endp = (T *)(buf + len);
				for (pp = (T *)buf; pp < endp; ++pp)
					val += *pp;
			}
			break;
		case CHECKSUM_CRC_CCITT:
			crc_ccitt_update(buf, len);
			break;
		case CHECKSUM_CRC32:
			crc_32_update(buf, len);
			break;
		}

		// Do any redrawing, but nothing else
		while (::PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_NOREMOVE))
		{
			if (::GetMessage(&msg, NULL, WM_PAINT, WM_PAINT))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}

        // Check if a key has been pressed
        if (::PeekMessage(&msg, NULL, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE))
        {
            // Windows does not like to miss key down events (need to match key up events)
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);

            // Remove any characters resulting from keypresses (so they are not inserted into the active file)
            while (::PeekMessage(&msg, NULL, WM_CHAR, WM_CHAR, PM_REMOVE))
                ;

            if (msg.wParam != 'y' &&  // avoid accidental multiple presses of Y key from aborting
                ::HMessageBox("Abort calculation?", MB_YESNO) == IDYES)
            {
				theApp.mac_error_ = 10;
				goto func_return;
            }
        }

		mm->m_wndStatusBar.SetPaneProgress(0, long(((curr - start_addr)*100)/(end_addr - start_addr)));
	}
	switch (op)
	{
	case CHECKSUM_CRC_CCITT:
		val = T(crc_ccitt_final());
		break;
	case CHECKSUM_CRC32:
		val = T(crc_32_final());
		break;
	}

    // Get final CRC and store it in the calculator
    if (((CMainFrame *)AfxGetMainWnd())->m_wndCalc.ByteSize() < sizeof(T))
        ((CMainFrame *)AfxGetMainWnd())->m_wndCalc.change_bits(sizeof(T)*8);
    ((CMainFrame *)AfxGetMainWnd())->m_wndCalc.Set(val);
    dynamic_cast<CMainFrame *>(::AfxGetMainWnd())->show_calc();          // make sure calc is displayed

	theApp.SaveToMacro(km_checksum, op);

func_return:
	mm->m_wndStatusBar.EnablePaneProgressBar(0, -1);  // disable progress bar

	if (buf != NULL)
		delete[] buf;
}

void CHexEditView::OnChecksum8()
{
	DoChecksum<unsigned char>(CHECKSUM_8, "8 bit Checksum");
}

void CHexEditView::OnChecksum16()
{
	DoChecksum<unsigned short>(CHECKSUM_16, "16 bit Checksum");
}

void CHexEditView::OnChecksum32()
{
	DoChecksum<unsigned long>(CHECKSUM_32, "32 bit Checksum");
}

void CHexEditView::OnChecksum64()
{
	DoChecksum<unsigned __int64>(CHECKSUM_64, "64 bit Checksum");
}

void CHexEditView::OnCrcCcitt() 
{
	DoChecksum<unsigned short>(CHECKSUM_CRC_CCITT, "CRC CCITT");
}

void CHexEditView::OnCrc32()
{
	DoChecksum<DWORD>(CHECKSUM_CRC32, "CRC 32");
}

void CHexEditView::OnMd5()
{
    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();
    unsigned char *buf = NULL;

    // Get current address or selection
    FILE_ADDRESS start_addr, end_addr;          // Start and end of selection
    GetSelAddr(start_addr, end_addr);

    if (start_addr >= end_addr)
    {
        // No selection, presumably in macro playback
        ASSERT(theApp.playing_);
        ::HMessageBox("There is no selection to calculate MD5 on");
        theApp.mac_error_ = 10;
        return;
    }
    ASSERT(start_addr < GetDocument()->length());

	// Get a buffer - fairly large for efficiency
	size_t len, buflen = size_t(min(4096, end_addr - start_addr));
	try
	{
        buf = new unsigned char[buflen];
    }
    catch(std::bad_alloc)
    {
        ::HMessageBox("Insufficient memory to perform MD5");
        theApp.mac_error_ = 10;
        return;
    }
	ASSERT(buf != NULL);
    mm->m_wndStatusBar.EnablePaneProgressBar(0, 100);  // turn on progress display
    MSG msg;                                           // use to check for abort key press

	struct MD5Context ctx;
	MD5Init(&ctx);
	for (FILE_ADDRESS curr = start_addr; curr < end_addr; curr += len)
	{
		// Get the next buffer full from the document
		len = size_t(min(buflen, end_addr - curr));
	    VERIFY(GetDocument()->GetData(buf, len, curr) == len);

		MD5Update(&ctx, buf, len);

		// Do any redrawing, but nothing else
		while (::PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_NOREMOVE))
		{
			if (::GetMessage(&msg, NULL, WM_PAINT, WM_PAINT))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}

        // Check if a key has been pressed
        if (::PeekMessage(&msg, NULL, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE))
        {
            // Windows does not like to miss key down events (need to match key up events)
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);

            // Remove any characters resulting from keypresses (so they are not inserted into the active file)
            while (::PeekMessage(&msg, NULL, WM_CHAR, WM_CHAR, PM_REMOVE))
                ;

            if (msg.wParam != 'y' &&  // avoid accidental multiple presses of Y key from aborting
                ::HMessageBox("Abort MD5 calculation?", MB_YESNO) == IDYES)
            {
				theApp.mac_error_ = 10;
				goto func_return;
            }
        }

		mm->m_wndStatusBar.SetPaneProgress(0, long(((curr - start_addr)*100)/(end_addr - start_addr)));
	}

	unsigned char digest[16];
    MD5Final(digest, &ctx);

	{
		// Display MD5 value (when calculator can handle 128 bits we will put it there instead)
		CString ss, fmt;
		if (theApp.hex_ucase_)
			fmt = "MD5: %2.2X%2.2X %2.2X%2.2X %2.2X%2.2X %2.2X%2.2X %2.2X%2.2X %2.2X%2.2X %2.2X%2.2X %2.2X%2.2X";
		else
			fmt = "MD5: %2.2x%2.2x %2.2x%2.2x %2.2x%2.2x %2.2x%2.2x %2.2x%2.2x %2.2x%2.2x %2.2x%2.2x %2.2x%2.2x";
		ss.Format(fmt,
				digest[0], digest[1], digest[2], digest[3],
				digest[4], digest[5], digest[6], digest[7],
				digest[8], digest[9], digest[10], digest[11],
				digest[12], digest[13], digest[14], digest[15]);
		::HMessageBox(ss);
	}

	// Record in macro since we did it successfully
	theApp.SaveToMacro(km_checksum, CHECKSUM_MD5);

func_return:
	mm->m_wndStatusBar.EnablePaneProgressBar(0, -1);  // disable progress bar

	if (buf != NULL)
		delete[] buf;
}

void CHexEditView::OnUpdateByteNZ(CCmdUI* pCmdUI) 
{
    FILE_ADDRESS start_addr, end_addr;              // Current selection
    GetSelAddr(start_addr, end_addr);

    if (pCmdUI->m_pSubMenu != NULL)
    {
        // This happens when popup menu itself is drawn
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION | (start_addr < end_addr ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
    }
    else
        // ??? is start_addr < end_addr then pos must be less than EOF ???
        pCmdUI->Enable(start_addr < end_addr && GetPos() < GetDocument()->length());
}

void CHexEditView::OnUpdate16bitNZ(CCmdUI* pCmdUI) 
{
    FILE_ADDRESS start_addr, end_addr;              // Current selection
    GetSelAddr(start_addr, end_addr);

    if (pCmdUI->m_pSubMenu != NULL)
    {
        // This happens when popup menu itself is drawn
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION | (start_addr < end_addr ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
    }
    else
        pCmdUI->Enable(end_addr > start_addr && (end_addr - start_addr)%2 == 0);
}

void CHexEditView::OnUpdate32bitNZ(CCmdUI* pCmdUI) 
{
    FILE_ADDRESS start_addr, end_addr;              // Current selection
    GetSelAddr(start_addr, end_addr);

    if (pCmdUI->m_pSubMenu != NULL)
    {
        // This happens when popup menu itself is drawn
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION | (start_addr < end_addr ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
    }
    else
        pCmdUI->Enable(end_addr > start_addr && (end_addr - start_addr)%4 == 0);
}

void CHexEditView::OnUpdate64bitNZ(CCmdUI* pCmdUI) 
{
    FILE_ADDRESS start_addr, end_addr;              // Current selection
    GetSelAddr(start_addr, end_addr);

    if (pCmdUI->m_pSubMenu != NULL)
    {
        // This happens when popup menu itself is drawn
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION | (start_addr < end_addr ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
    }
    else
        pCmdUI->Enable(end_addr > start_addr && (end_addr - start_addr)%8 == 0);
}

template<class T> void ProcBinary(CHexEditView *pv, T val, T *buf, size_t count, binop_type op, int &div0)
{
    for (size_t ii = 0; ii < count; ++ii)
    {
        // Reverse byte order if using big-endian
        if (pv->BigEndian())
            ::flip_bytes((unsigned char *)&buf[ii], sizeof(T));
        switch (op)
        {
        case binop_add:
            buf[ii] += val;
            break;
        case binop_subtract:
            buf[ii] -= val;
            break;
        case binop_subtract_x:
            buf[ii] = val - buf[ii];
            break;
        case binop_multiply:
            buf[ii] *= val;
            break;
        case binop_divide:
            ASSERT(val != 0);
            buf[ii] /= val;
            break;
        case binop_divide_x:
            if (buf[ii] != 0)
                buf[ii] = val / buf[ii];
            else
                ++div0;
            break;
        case binop_mod:
            ASSERT(val != 0);
            buf[ii] %= val;
            break;
        case binop_mod_x:
            if (buf[ii] != 0)
                buf[ii] = val % buf[ii];
            else
                ++div0;
            break;
        case binop_and:
            buf[ii] &= val;
            break;
        case binop_or:
            buf[ii] |= val;
            break;
        case binop_xor:
            buf[ii] ^= val;
            break;

        // Signed/unsigned comparisons are differentiated by type of T passed to the template function
        case binop_gtr:
        case binop_gtr_unsigned:
            if (val > buf[ii])
                buf[ii] = val;
            break;
        case binop_less:
        case binop_less_unsigned:
            if (val < buf[ii])
                buf[ii] = val;
            break;

        case binop_rol:  // T must be unsigned type for zero fill in >>
            {
                int tmp = int(val) % (sizeof(T)*8);
                buf[ii] = (buf[ii] << tmp) | (buf[ii] >> (sizeof(T)*8 - tmp));
            }
            break;
        case binop_ror:  // T must be unsigned type for zero fill in >>
            {
                int tmp = int(val) % (sizeof(T)*8);
                buf[ii] = (buf[ii] >> tmp) | (buf[ii] << (sizeof(T)*8 - tmp));
            }
            break;
        case binop_lsl:
            buf[ii] <<= int(val);
            break;

        case binop_lsr:  // T must be unsigned type for zero fill
        case binop_asr:  // T must be signed type for sign to be extended
            buf[ii] >>= int(val);
            break;
        default:
            ASSERT(0);
        }
        // Reverse byte order back again before putting back to the file
        if (pv->BigEndian())
            ::flip_bytes((unsigned char *)&buf[ii], sizeof(T));
    }
}

// Perform binary operations (where the 2nd operand is taken from the calculator).
// [This is used to replace numerous other functions (OnAddByte, OnXor64Bit etc).]
// T = template parameter specifying size of operand (1,2,4 or 8 byte integer)
// pv = pointer to the view (we had to do this as VC6 does not support member templates)
// op = operation to perform
// desc = describes the operations and operand size for use in error messages
// dummy = determines template operand type (should not be nec. except for VC6 template bugs)
template<class T> void OnOperateBinary(CHexEditView *pv, binop_type op, LPCSTR desc, T dummy) 
{
    (void)dummy;  // The dummy param. makes sure we get the right template (VC++ 6 bug)

    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();
    unsigned char *buf = NULL;

    ASSERT(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);
    if (pv->check_ro(desc))
        return;

    if (mm->m_wndCalc.ByteSize() > sizeof(T))
    {
        ASSERT(theApp.playing_);  // This should only happen during playback since commands are disabled otherwise
        CString mess;
        mess.Format("Can't %s unless operand (calculator) is %d bytes or less", desc, sizeof(T));
        ::HMessageBox(mess);
        theApp.mac_error_ = 10;
        return;
    }

    // Get current address of selection
    FILE_ADDRESS start_addr, end_addr;          // Start and end of selection
    pv->GetSelAddr(start_addr, end_addr);

    // If no selection just do one
    if (start_addr >= end_addr)
	{
        end_addr = start_addr + sizeof(T);
		if (end_addr > pv->GetDocument()->length())
		{
			// Not enough bytes before EOF, presumably in macro playback
			ASSERT(theApp.playing_);
			CString mess;
			mess.Format("Insufficient bytes before EOF to %s", desc);
			::HMessageBox(mess);
			theApp.mac_error_ = 10;
			return;
		}
    }

	// Make sure selection is a multiple of the data type length
    if ((end_addr - start_addr) % sizeof(T) != 0)
    {
        ASSERT(theApp.playing_);
        CString mess;
        mess.Format("Selection must be a multiple of %d bytes to %s", sizeof(T), desc);
        ::HMessageBox(mess);
        theApp.mac_error_ = 10;
        return;
    }
    ASSERT(start_addr < end_addr && end_addr <= pv->GetDocument()->length());

	// Get the other operand from the calculator
    T val = T(mm->m_wndCalc.GetValue());

    // Check for calculator val of zero causing divide by zero
    if (val == 0 && (op == binop_divide || op == binop_mod))
    {
        CString mess;
        mess.Format("Calculator value is zero (causing divide by 0) while performing %s", desc);
        ::HMessageBox(mess);
        theApp.mac_error_ = 5;
        return;
    }

    int div0 = 0;   // Count of divide by zero errors (for binop_divide_x and binop_mod_x)

	// Test if selection is too big to do in memory
	if (end_addr - start_addr > (16*1024*1024) && pv->GetDocument()->DataFileSlotFree())
	{
        int idx = -1;                       // Index into docs data_file_ array (or -1 if no slots avail.)

		// Create a file to store the resultant data
		// (Temp file used by the document until it is closed or written to disk.)
		char temp_dir[_MAX_PATH];
		char temp_file[_MAX_PATH];
		::GetTempPath(sizeof(temp_dir), temp_dir);
		::GetTempFileName(temp_dir, _T("_HE"), 0, temp_file);

		// Get data buffer
		size_t len, buflen = size_t(min(4096, end_addr - start_addr));
		try
		{
            buf = new unsigned char[buflen];
        }
        catch(std::bad_alloc)
        {
            ::HMessageBox("Insufficient memory to perform operation");
            theApp.mac_error_ = 10;
            goto func_return;
        }
        mm->m_wndStatusBar.EnablePaneProgressBar(0, 100);  // turn on progress display
        MSG msg;                                           // use to check for abort key press

		try
		{
		    CFile64 ff(temp_file, CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive|CFile::typeBinary);

			for (FILE_ADDRESS curr = start_addr; curr < end_addr; curr += len)
			{
				// Get the next buffer full from the document
				len = size_t(min(buflen, end_addr - curr));
				VERIFY(pv->GetDocument()->GetData(buf, len, curr) == len);

				ProcBinary(pv, val, (T *)buf, len/sizeof(T), op, div0);

				ff.Write(buf, len);

				// Do any redrawing, but nothing else
				while (::PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_NOREMOVE))
				{
					if (::GetMessage(&msg, NULL, WM_PAINT, WM_PAINT))
					{
						::TranslateMessage(&msg);
						::DispatchMessage(&msg);
					}
				}

                // Check if a key has been pressed
                if (::PeekMessage(&msg, NULL, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE))
                {
                    // Windows does not like to miss key down events (need to match key up events)
                    ::TranslateMessage(&msg);
                    ::DispatchMessage(&msg);

                    // Remove any characters resulting from keypresses (so they are not inserted into the active file)
                    while (::PeekMessage(&msg, NULL, WM_CHAR, WM_CHAR, PM_REMOVE))
                        ;

					CString mess;
					mess.Format("Abort %s operation?", desc);
                    if (msg.wParam != 'y' &&  // avoid accidental multiple presses of Y key from aborting
                        ::HMessageBox(mess, MB_YESNO) == IDYES)
                    {
						ff.Close();
						remove(temp_file);
						theApp.mac_error_ = 10;
						goto func_return;
                    }
                }

				mm->m_wndStatusBar.SetPaneProgress(0, long(((curr - start_addr)*100)/(end_addr - start_addr)));
			}
		}
		catch (CFileException *pfe)
		{
			::HMessageBox(::FileErrorMessage(pfe, CFile::modeWrite));
			pfe->Delete();

			remove(temp_file);
			theApp.mac_error_ = 10;
			goto func_return;
		}

		// Delete the unprocessed block that is to be replaced
		// Note: this must be done before AddDataFile otherwise Change() (via regenerate()) will delete the temp file.
        pv->GetDocument()->Change(mod_delforw, start_addr, end_addr - start_addr, NULL, 0, pv);

		// Add the temp file to the document
		idx = pv->GetDocument()->AddDataFile(temp_file);
		ASSERT(idx != -1);
        pv->GetDocument()->Change(mod_insert_file, start_addr, end_addr - start_addr, NULL, idx, pv, TRUE);
	}
	else
	{
		// Allocate memory block and process it

		// Error if ran out of temp file handles and file is too big
		if (end_addr - start_addr > UINT_MAX)  // why is there no SIZE_T_MAX?
		{
			::HMessageBox("HexEdit is out of temporary files and \n"
                          "cannot operate on such a large selection. \n"
                          "Please save the file to deallocate \n"
						  "temporary file handles and try again.\n",
						  MB_OK);
            theApp.mac_error_ = 10;
			return;
		}

		// Warn if we might cause memory exhaustion
        if (end_addr - start_addr > 128*1024*1024)  // 128 Mb file may be too big
        {
			if (::HMessageBox("WARNING: HexEdit is out of temporary file \n"
                              "handles and operating on such a large selection \n"
                              "may cause memory exhaustion.  Please click NO \n"
                              "and save the file to free handles. \n"
							  "Or, click YES to continue.\n\n"
                              "Do you want to continue?", MB_YESNO) != IDYES)
			{
                theApp.mac_error_ = 5;
                return;
			}
        }

        CWaitCursor wait;                           // Turn on wait cursor (hourglass)

        try
        {
            buf = new unsigned char[size_t(end_addr - start_addr)];
        }
        catch(std::bad_alloc)
        {
            ::HMessageBox("Insufficient memory to perform operation");
            theApp.mac_error_ = 10;
            goto func_return;
        }
        size_t got = pv->GetDocument()->GetData(buf, size_t(end_addr - start_addr), start_addr);
        ASSERT(got == size_t(end_addr - start_addr));

		ProcBinary(pv, val, (T *)buf, got/sizeof(T), op, div0);

        pv->GetDocument()->Change(mod_replace, start_addr, got, (unsigned char *)buf, 0, pv);
	}

    if (div0 > 0)
    {
        // Tell the user about divide by zero errors
        CString mess;
        mess.Format("There were %d divide by zero errors while performing %s", int(div0), desc);
        ::HMessageBox(mess);
        theApp.mac_error_ = 5;
    }

    pv->DisplayCaret();

	{
		static int bit_pos[8] = {0, 1, -1, 2, -1, -1, -1, 3};
		ASSERT(sizeof(T)-1 < 8 && bit_pos[sizeof(T)-1] != -1);  // size must be 1,2,4,8 giving bit_pos of 0,1,2,3
		theApp.SaveToMacro(km_op_binary, (op<<8)|bit_pos[sizeof(T)-1]);
	}

func_return:
	mm->m_wndStatusBar.EnablePaneProgressBar(0, -1);  // disable progress bar

	if (buf != NULL)
		delete[] buf;
}

void CHexEditView::OnAddByte() 
{
    ::OnOperateBinary<char>(this, binop_add, "Add bytes", char(0));
}

void CHexEditView::OnAdd16bit() 
{
    ::OnOperateBinary<short>(this, binop_add, "Add words", short(0));
}

void CHexEditView::OnAdd32bit() 
{
    ::OnOperateBinary<long>(this, binop_add, "Add double words", long(0));
}

void CHexEditView::OnAdd64bit() 
{
    ::OnOperateBinary<__int64>(this, binop_add, "Add quad words", __int64(0));
}

void CHexEditView::OnSubtractByte() 
{
    ::OnOperateBinary<char>(this, binop_subtract, "Subtract bytes", char(0));
}

void CHexEditView::OnSubtract16bit() 
{
    ::OnOperateBinary<short>(this, binop_subtract, "Subtract words", short(0));
}

void CHexEditView::OnSubtract32bit() 
{
    ::OnOperateBinary<long>(this, binop_subtract, "Subtract double words", long(0));
}

void CHexEditView::OnSubtract64bit() 
{
    ::OnOperateBinary<__int64>(this, binop_subtract, "Subtract quad words", __int64(0));
}

void CHexEditView::OnAndByte() 
{
    ::OnOperateBinary<char>(this, binop_and, "AND bytes", char(0));
}

void CHexEditView::OnAnd16bit() 
{
    ::OnOperateBinary<short>(this, binop_and, "AND words", short(0));
}

void CHexEditView::OnAnd32bit() 
{
    ::OnOperateBinary<long>(this, binop_and, "AND double words", long(0));
}

void CHexEditView::OnAnd64bit() 
{
    ::OnOperateBinary<__int64>(this, binop_and, "AND quad words", __int64(0));
}

void CHexEditView::OnOrByte() 
{
    ::OnOperateBinary<char>(this, binop_or, "OR bytes", char(0));
}

void CHexEditView::OnOr16bit() 
{
    ::OnOperateBinary<short>(this, binop_or, "OR words", short(0));
}

void CHexEditView::OnOr32bit() 
{
    ::OnOperateBinary<long>(this, binop_or, "OR double words", long(0));
}

void CHexEditView::OnOr64bit() 
{
    ::OnOperateBinary<__int64>(this, binop_or, "OR quad words", __int64(0));
}

void CHexEditView::OnXorByte() 
{
    ::OnOperateBinary<char>(this, binop_xor, "XOR bytes", char(0));
}

void CHexEditView::OnXor16bit() 
{
    ::OnOperateBinary<short>(this, binop_xor, "XOR words", short(0));
}

void CHexEditView::OnXor32bit() 
{
    ::OnOperateBinary<long>(this, binop_xor, "XOR double words", long(0));
}

void CHexEditView::OnXor64bit() 
{
    ::OnOperateBinary<__int64>(this, binop_xor, "XOR quad words", __int64(0));
}

void CHexEditView::OnMulByte() 
{
    ::OnOperateBinary<unsigned char>(this, binop_multiply, "multiply bytes", unsigned char(0));
}

void CHexEditView::OnMul16bit() 
{
    ::OnOperateBinary<unsigned short>(this, binop_multiply, "multiply words", unsigned short(0));
}

void CHexEditView::OnMul32bit() 
{
    ::OnOperateBinary<unsigned long>(this, binop_multiply, "multiply double words", unsigned long(0));
}

void CHexEditView::OnMul64bit() 
{
    ::OnOperateBinary<unsigned __int64>(this, binop_multiply, "multiply quad words", unsigned __int64(0));
}

void CHexEditView::OnDivByte() 
{
    ::OnOperateBinary<unsigned char>(this, binop_divide, "divide bytes", unsigned char(0));
}

void CHexEditView::OnDiv16bit() 
{
    ::OnOperateBinary<unsigned short>(this, binop_divide, "divide words", unsigned short(0));
}

void CHexEditView::OnDiv32bit() 
{
    ::OnOperateBinary<unsigned long>(this, binop_divide, "divide double words", unsigned long(0));
}

void CHexEditView::OnDiv64bit() 
{
    ::OnOperateBinary<unsigned __int64>(this, binop_divide, "divide quad words", unsigned __int64(0));
}

void CHexEditView::OnModByte() 
{
    ::OnOperateBinary<unsigned char>(this, binop_mod, "modulus bytes", unsigned char(0));
}

void CHexEditView::OnMod16bit() 
{
    ::OnOperateBinary<unsigned short>(this, binop_mod, "modulus words", unsigned short(0));
}

void CHexEditView::OnMod32bit() 
{
    ::OnOperateBinary<unsigned long>(this, binop_mod, "modulus double words", unsigned long(0));
}

void CHexEditView::OnMod64bit() 
{
    ::OnOperateBinary<unsigned __int64>(this, binop_mod, "modulus quad words", unsigned __int64(0));
}

void CHexEditView::OnSubtractXByte() 
{
    ::OnOperateBinary<char>(this, binop_subtract_x, "Subtract bytes", char(0));
}

void CHexEditView::OnSubtractX16bit() 
{
    ::OnOperateBinary<short>(this, binop_subtract_x, "Subtract words", short(0));
}

void CHexEditView::OnSubtractX32bit() 
{
    ::OnOperateBinary<long>(this, binop_subtract_x, "Subtract double words", long(0));
}

void CHexEditView::OnSubtractX64bit() 
{
    ::OnOperateBinary<__int64>(this, binop_subtract_x, "Subtract quad words", __int64(0));
}

void CHexEditView::OnDivXByte() 
{
    ::OnOperateBinary<unsigned char>(this, binop_divide_x, "divide bytes", unsigned char(0));
}

void CHexEditView::OnDivX16bit() 
{
    ::OnOperateBinary<unsigned short>(this, binop_divide_x, "divide words", unsigned short(0));
}

void CHexEditView::OnDivX32bit() 
{
    ::OnOperateBinary<unsigned long>(this, binop_divide_x, "divide double words", unsigned long(0));
}

void CHexEditView::OnDivX64bit() 
{
    ::OnOperateBinary<unsigned __int64>(this, binop_divide_x, "divide quad words", unsigned __int64(0));
}

void CHexEditView::OnModXByte() 
{
    ::OnOperateBinary<unsigned char>(this, binop_mod_x, "modulus bytes", unsigned char(0));
}

void CHexEditView::OnModX16bit() 
{
    ::OnOperateBinary<unsigned short>(this, binop_mod_x, "modulus words", unsigned short(0));
}

void CHexEditView::OnModX32bit() 
{
    ::OnOperateBinary<unsigned long>(this, binop_mod_x, "modulus double words", unsigned long(0));
}

void CHexEditView::OnModX64bit() 
{
    ::OnOperateBinary<unsigned __int64>(this, binop_mod_x, "modulus quad words", unsigned __int64(0));
}

void CHexEditView::OnGtrByte() 
{
    ::OnOperateBinary<signed char>(this, binop_gtr, "finding greater of bytes", signed char(0));
}

void CHexEditView::OnGtr16bit() 
{
    ::OnOperateBinary<short>(this, binop_gtr, "finding greater of words", short(0));
}

void CHexEditView::OnGtr32bit() 
{
    ::OnOperateBinary<long>(this, binop_gtr, "finding greater of double words", long(0));
}

void CHexEditView::OnGtr64bit() 
{
    ::OnOperateBinary<__int64>(this, binop_gtr, "finding greater of quad words", __int64(0));
}

void CHexEditView::OnLessByte() 
{
    ::OnOperateBinary<signed char>(this, binop_less, "finding lesser of bytes", signed char(0));
}

void CHexEditView::OnLess16bit() 
{
    ::OnOperateBinary<short>(this, binop_less, "finding lesser of words", short(0));
}

void CHexEditView::OnLess32bit() 
{
    ::OnOperateBinary<long>(this, binop_less, "finding lesser of double words", long(0));
}

void CHexEditView::OnLess64bit() 
{
    ::OnOperateBinary<__int64>(this, binop_less, "finding lesser of quad words", __int64(0));
}

void CHexEditView::OnGtrUByte() 
{
    ::OnOperateBinary<unsigned char>(this, binop_gtr_unsigned, "finding unsigned greater of bytes", unsigned char(0));
}

void CHexEditView::OnGtrU16bit() 
{
    ::OnOperateBinary<unsigned short>(this, binop_gtr_unsigned, "finding unsigned greater of words", unsigned short(0));
}

void CHexEditView::OnGtrU32bit() 
{
    ::OnOperateBinary<unsigned long>(this, binop_gtr_unsigned, "finding unsigned greater of double words", unsigned long(0));
}

void CHexEditView::OnGtrU64bit() 
{
    ::OnOperateBinary<unsigned __int64>(this, binop_gtr_unsigned, "finding unsigned greater of quad words", unsigned __int64(0));
}

void CHexEditView::OnLessUByte() 
{
    ::OnOperateBinary<unsigned char>(this, binop_less_unsigned, "finding unsigned lesser of bytes", unsigned char(0));
}

void CHexEditView::OnLessU16bit() 
{
    ::OnOperateBinary<unsigned short>(this, binop_less_unsigned, "finding unsigned lesser of words", unsigned short(0));
}

void CHexEditView::OnLessU32bit() 
{
    ::OnOperateBinary<unsigned long>(this, binop_less_unsigned, "finding unsigned lesser of double words", unsigned long(0));
}

void CHexEditView::OnLessU64bit() 
{
    ::OnOperateBinary<unsigned __int64>(this, binop_less_unsigned, "finding unsigned lesser of quad words", unsigned __int64(0));
}

void CHexEditView::OnRolByte() 
{
    ::OnOperateBinary<unsigned char>(this, binop_rol, "rotate left bytes", unsigned char(0));
}

void CHexEditView::OnRol16bit() 
{
    ::OnOperateBinary<unsigned short>(this, binop_rol, "rotate left words", unsigned short(0));
}

void CHexEditView::OnRol32bit() 
{
    ::OnOperateBinary<unsigned long>(this, binop_rol, "rotate left double words", unsigned long(0));
}

void CHexEditView::OnRol64bit() 
{
    ::OnOperateBinary<unsigned __int64>(this, binop_rol, "rotate left quad words", unsigned __int64(0));
}

void CHexEditView::OnRorByte() 
{
    ::OnOperateBinary<unsigned char>(this, binop_ror, "rotate right bytes", unsigned char(0));
}

void CHexEditView::OnRor16bit() 
{
    ::OnOperateBinary<unsigned short>(this, binop_ror, "rotate right words", unsigned short(0));
}

void CHexEditView::OnRor32bit() 
{
    ::OnOperateBinary<unsigned long>(this, binop_ror, "rotate right double words", unsigned long(0));
}

void CHexEditView::OnRor64bit() 
{
    ::OnOperateBinary<unsigned __int64>(this, binop_ror, "rotate right quad words", unsigned __int64(0));
}

void CHexEditView::OnLslByte() 
{
    ::OnOperateBinary<char>(this, binop_lsl, "shift left bytes", char(0));
}

void CHexEditView::OnLsl16bit() 
{
    ::OnOperateBinary<short>(this, binop_lsl, "shift left words", short(0));
}

void CHexEditView::OnLsl32bit() 
{
    ::OnOperateBinary<long>(this, binop_lsl, "shift left double words", long(0));
}

void CHexEditView::OnLsl64bit() 
{
    ::OnOperateBinary<__int64>(this, binop_lsl, "shift left quad words", __int64(0));
}

void CHexEditView::OnLsrByte() 
{
    ::OnOperateBinary<unsigned char>(this, binop_lsr, "shift right bytes", unsigned char(0));
}

void CHexEditView::OnLsr16bit() 
{
    ::OnOperateBinary<unsigned short>(this, binop_lsr, "shift right words", unsigned short(0));
}

void CHexEditView::OnLsr32bit() 
{
    ::OnOperateBinary<unsigned long>(this, binop_lsr, "shift right double words", unsigned long(0));
}

void CHexEditView::OnLsr64bit() 
{
    ::OnOperateBinary<unsigned __int64>(this, binop_lsr, "shift right quad words", unsigned __int64(0));
}

void CHexEditView::OnAsrByte() 
{
    ::OnOperateBinary<signed char>(this, binop_asr, "arithmetic shift right bytes", signed char(0));
}

void CHexEditView::OnAsr16bit() 
{
    ::OnOperateBinary<short>(this, binop_asr, "arithmetic shift right words", short(0));
}

void CHexEditView::OnAsr32bit() 
{
    ::OnOperateBinary<long>(this, binop_asr, "arithmetic shift right double words", long(0));
}

void CHexEditView::OnAsr64bit() 
{
    ::OnOperateBinary<__int64>(this, binop_asr, "arithmetic shift right quad words", __int64(0));
}

template<class T> void ProcUnary(CHexEditView *pv, T val, T *buf, size_t count, unary_type op)
{
    // Operate on all the selected values
    for (size_t ii = 0; ii < count; ++ii)
    {
        // Reverse if big endian and the operation calls for it.
        if (pv->BigEndian() && op != unary_flip)
            ::flip_bytes((unsigned char *)&buf[ii], sizeof(T));
        switch (op)
        {
        case unary_at:
            buf[ii] = val;
            break;
        case unary_sign:
            buf[ii] = -buf[ii];
            break;
        case unary_inc:
            ++buf[ii];
            if (buf[ii] == 0)
            {
                ((CMainFrame *)AfxGetMainWnd())->StatusBarText("Warning: Increment wrapped around to zero");
                theApp.mac_error_ = 1;            // Signal warning on wrap
            }
            break;
        case unary_dec:
            buf[ii]--;
            if (buf[ii] == -1)
            {
                ((CMainFrame *)AfxGetMainWnd())->StatusBarText("Warning: Decrement wrapped past zero");
                theApp.mac_error_ = 1;            // Signal warning on wrap
            }
            break;
        case unary_not:
            ASSERT(sizeof(T) == 1);  // Only need to perform NOT on bytes
            buf[ii] = ~buf[ii];
            break;
        case unary_flip:
            ASSERT(sizeof(T) > 1);   // No point in flipping a single byte
            // Do nothing here (let ::flip_bytes below handle it)
            break;
        case unary_rev:
            {
                T result = 0;
                T tmp = buf[ii];
                for (int jj = 0; jj < sizeof(T)*8; ++jj)
                {
                    result <<= 1;                      // Make room for the next bit
                    if (tmp & 0x1)
                        result |= 0x1;
                    tmp >>= 1;                         // Move bits down to test the next
                }
                buf[ii] = result;
            }
            break;
        case unary_rand:
            ASSERT(sizeof(T) == 1);  // Only really need random bytes
            buf[ii] = T(::rand_good()>>16);
            break;
        case unary_rand_fast:
            ASSERT(sizeof(T) == 1);  // Only really need random bytes
            buf[ii] = T(::rand()>>7);
            break;
        default:
            ASSERT(0);
        }
        if (pv->BigEndian() || op == unary_flip)
            ::flip_bytes((unsigned char *)&buf[ii], sizeof(T));
    }
}

// Perform unary operations on the selection
// [This is used to replace numerous other functions (OnIncByte, OnFlip64Bit etc).]
// T = template parameter specifying size of operand (1,2,4 or 8 byte integer)
// op = operation to perform
// desc = describes the operations and operand size for use in error messages
template<class T> void OnOperateUnary(CHexEditView *pv, unary_type op, LPCSTR desc, T dummy)
{
    (void)dummy;  // The dummy param. makes sure we get the right template (VC++ 6 bug)
    CMainFrame *mm = (CMainFrame *)AfxGetMainWnd();
    unsigned char *buf = NULL;

    ASSERT(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);
    if (pv->check_ro(desc))
        return;

    T val = 0;
    // For assign operation we get the value from calc
    if (op == unary_at)
    {
        val = T(mm->m_wndCalc.GetValue());  // prefetch the value

		// Make sure the operand is not too big
		if (mm->m_wndCalc.ByteSize() > sizeof(T))
		{
			ASSERT(theApp.playing_);  // This should only happen during playback since commands are disabled otherwise
			CString mess;
			mess.Format("Can't %s unless operand (calculator) is %d bytes or less", desc, sizeof(T));
			::HMessageBox(mess);
			theApp.mac_error_ = 10;
			return;
		}
    }

    // Get current address or selection
    FILE_ADDRESS start_addr, end_addr;          // Start and end of selection
    pv->GetSelAddr(start_addr, end_addr);

    // If no selection just do one
    if (start_addr >= end_addr)
	{
        end_addr = start_addr + sizeof(T);
		if (end_addr > pv->GetDocument()->length())
		{
			// Not enough bytes before EOF, presumably in macro playback
			ASSERT(theApp.playing_);
			CString mess;
			mess.Format("Insufficient bytes before EOF to %s", desc);
			::HMessageBox(mess);
			theApp.mac_error_ = 10;
			return;
		}
    }

	// Make sure selection is a multiple of the data type length
    if ((end_addr - start_addr) % sizeof(T) != 0)
    {
        ASSERT(theApp.playing_);
        CString mess;
        mess.Format("Selection must be a multiple of %d bytes to %s", sizeof(T), desc);
        ::HMessageBox(mess);
        theApp.mac_error_ = 10;
        return;
    }
    ASSERT(start_addr < end_addr && end_addr <= pv->GetDocument()->length());

	// Test if selection is too big to do in memory
	if (end_addr - start_addr > (16*1024*1024) && pv->GetDocument()->DataFileSlotFree())
	{
        int idx = -1;                       // Index into docs data_file_ array (or -1 if no slots avail.)

		// Create a file to store the resultant data
		// (Temp file used by the document until it is closed or written to disk.)
		char temp_dir[_MAX_PATH];
		char temp_file[_MAX_PATH];
		::GetTempPath(sizeof(temp_dir), temp_dir);
		::GetTempFileName(temp_dir, _T("_HE"), 0, temp_file);

		// Get data buffer
		size_t len, buflen = size_t(min(4096, end_addr - start_addr));
		try
		{
            buf = new unsigned char[buflen];
        }
        catch(std::bad_alloc)
        {
            ::HMessageBox("Insufficient memory to perform operation");
            theApp.mac_error_ = 10;
            goto func_return;
        }
        mm->m_wndStatusBar.EnablePaneProgressBar(0, 100);  // turn on progress display
        MSG msg;                                           // use to check for abort key press

		try
		{
		    CFile64 ff(temp_file, CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive|CFile::typeBinary);

			for (FILE_ADDRESS curr = start_addr; curr < end_addr; curr += len)
			{
				// Get the next buffer full from the document
				len = size_t(min(buflen, end_addr - curr));
				if (op != unary_at)      // We don't need to get old value if assigning
				    VERIFY(pv->GetDocument()->GetData(buf, len, curr) == len);

				ProcUnary(pv, val, (T *)buf, len/sizeof(T), op);

				ff.Write(buf, len);

				// Do any redrawing, but nothing else
				while (::PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_NOREMOVE))
				{
					if (::GetMessage(&msg, NULL, WM_PAINT, WM_PAINT))
					{
						::TranslateMessage(&msg);
						::DispatchMessage(&msg);
					}
				}

                // Check if a key has been pressed
                if (::PeekMessage(&msg, NULL, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE))
                {
                    // Windows does not like to miss key down events (need to match key up events)
                    ::TranslateMessage(&msg);
                    ::DispatchMessage(&msg);

                    // Remove any characters resulting from keypresses (so they are not inserted into the active file)
                    while (::PeekMessage(&msg, NULL, WM_CHAR, WM_CHAR, PM_REMOVE))
                        ;

					CString mess;
					mess.Format("Abort %s operation?", desc);
                    if (msg.wParam != 'y' &&  // avoid accidental multiple presses of Y key from aborting
                        ::HMessageBox(mess, MB_YESNO) == IDYES)
                    {
						ff.Close();
						remove(temp_file);
						theApp.mac_error_ = 10;
						goto func_return;
                    }
                }

				mm->m_wndStatusBar.SetPaneProgress(0, long(((curr - start_addr)*100)/(end_addr - start_addr)));
			}
		}
		catch (CFileException *pfe)
		{
			::HMessageBox(::FileErrorMessage(pfe, CFile::modeWrite));
			pfe->Delete();

			remove(temp_file);
			theApp.mac_error_ = 10;
			goto func_return;
		}

		// Delete the unprocessed block that is to be replaced
		// Note: this must be done before AddDataFile otherwise Change() (via regenerate()) will delete the temp file.
        pv->GetDocument()->Change(mod_delforw, start_addr, end_addr - start_addr, NULL, 0, pv);

		// Add the temp file to the document
		idx = pv->GetDocument()->AddDataFile(temp_file);
		ASSERT(idx != -1);
        pv->GetDocument()->Change(mod_insert_file, start_addr, end_addr - start_addr, NULL, idx, pv, TRUE);
	}
	else
	{
		// Allocate memory block and process it

		// Error if ran out of temp file handles and file is too big
		if (end_addr - start_addr > UINT_MAX)  // why is there no SIZE_T_MAX?
		{
			::HMessageBox("HexEdit is out of temporary files and \n"
                          "cannot operate on such a large selection. \n"
                          "Please save the file to deallocate \n"
						  "temporary file handles and try again.\n",
						  MB_OK);
            theApp.mac_error_ = 10;
			return;
		}

		// Warn if we might cause memory exhaustion
        if (end_addr - start_addr > 128*1024*1024)  // 128 Mb file may be too big
        {
			if (::HMessageBox("WARNING: HexEdit is out of temporary file \n"
                              "handles and operating on such a large selection \n"
                              "may cause memory exhaustion.  Please click NO \n"
                              "and save the file to free handles. \n"
							  "Or, click YES to continue.\n\n"
                              "Do you want to continue?", MB_YESNO) != IDYES)
			{
                theApp.mac_error_ = 5;
                return;
			}
        }

        CWaitCursor wait;                           // Turn on wait cursor (hourglass)

        try
        {
            buf = new unsigned char[size_t(end_addr - start_addr)];
        }
        catch(std::bad_alloc)
        {
            ::HMessageBox("Insufficient memory to perform operation");
            theApp.mac_error_ = 10;
            goto func_return;
        }
		size_t got;
		if (op != unary_at)      // We don't need to get old value if assigning
		{
			got = pv->GetDocument()->GetData(buf, size_t(end_addr - start_addr), start_addr);
			ASSERT(got == size_t(end_addr - start_addr));
		}
		else
			got = size_t(end_addr - start_addr);

		ProcUnary(pv, val, (T *)buf, got/sizeof(T), op);

        pv->GetDocument()->Change(mod_replace, start_addr, got, (unsigned char *)buf, 0, pv);
	}

    pv->DisplayCaret();

	{
		static int bit_pos[8] = {0, 1, -1, 2, -1, -1, -1, 3};
		ASSERT(sizeof(T)-1 < 8 && bit_pos[sizeof(T)-1] != -1);  // size must be 1,2,4,8 giving bit_pos of 0,1,2,3
		theApp.SaveToMacro(km_op_unary, (op<<8)|bit_pos[sizeof(T)-1]);
	}

func_return:
	mm->m_wndStatusBar.EnablePaneProgressBar(0, -1);  // disable progress bar

	if (buf != NULL)
		delete[] buf;
}

void CHexEditView::OnIncByte() 
{
    ::OnOperateUnary<char>(this, unary_inc, "increment bytes", char(0));
}

void CHexEditView::OnInc16bit() 
{
    ::OnOperateUnary<short>(this, unary_inc, "increment words", short(0));
}

void CHexEditView::OnInc32bit() 
{
    ::OnOperateUnary<long>(this, unary_inc, "increment double words", long(0));
}

void CHexEditView::OnInc64bit() 
{
    ::OnOperateUnary<__int64>(this, unary_inc, "increment quad words", __int64(0));
}

void CHexEditView::OnDecByte() 
{
    ::OnOperateUnary<char>(this, unary_dec, "decrement bytes", char(0));
}

void CHexEditView::OnDec16bit() 
{
    ::OnOperateUnary<short>(this, unary_dec, "decrement words", short(0));
}

void CHexEditView::OnDec32bit() 
{
    ::OnOperateUnary<long>(this, unary_dec, "decrement double words", long(0));
}

void CHexEditView::OnDec64bit() 
{
    ::OnOperateUnary<__int64>(this, unary_dec, "decrement quad words", __int64(0));
}

void CHexEditView::OnRevByte() 
{
    ::OnOperateUnary<char>(this, unary_rev, "reverse bits of bytes", char(0));
}

void CHexEditView::OnRev16bit() 
{
    ::OnOperateUnary<short>(this, unary_rev, "reverse bits of words", short(0));
}

void CHexEditView::OnRev32bit() 
{
    ::OnOperateUnary<long>(this, unary_rev, "reverse bits of double words", long(0));
}

void CHexEditView::OnRev64bit() 
{
    ::OnOperateUnary<__int64>(this, unary_rev, "reverse bits of quad words", __int64(0));
}

// There is NO OnFlipByte (since it would not do anything)

void CHexEditView::OnFlip16bit() 
{
    ::OnOperateUnary<short>(this, unary_flip, "flip bytes of words", short(0));
}

void CHexEditView::OnFlip32bit() 
{
    ::OnOperateUnary<long>(this, unary_flip, "flip bytes of double words", long(0));
}

void CHexEditView::OnFlip64bit() 
{
    ::OnOperateUnary<__int64>(this, unary_flip, "flip bytes of quad words", __int64(0));
}

void CHexEditView::OnInvert() 
{
    ::OnOperateUnary<char>(this, unary_not, "invert bits of bytes", char(0));
}

void CHexEditView::OnNegByte() 
{
    ::OnOperateUnary<signed char>(this, unary_sign, "negate bytes", signed char(0));
}

void CHexEditView::OnNeg16bit() 
{
    ::OnOperateUnary<short>(this, unary_sign, "negate words", short(0));
}

void CHexEditView::OnNeg32bit() 
{
    ::OnOperateUnary<long>(this, unary_sign, "negate double words", long(0));
}

void CHexEditView::OnNeg64bit() 
{
    ::OnOperateUnary<__int64>(this, unary_sign, "negate quad words", __int64(0));
}

void CHexEditView::OnAssignByte() 
{
    ::OnOperateUnary<char>(this, unary_at, "assign bytes", char(0));
}

void CHexEditView::OnAssign16bit() 
{
    ::OnOperateUnary<short>(this, unary_at, "assign words", short(0));
}

void CHexEditView::OnAssign32bit() 
{
    ::OnOperateUnary<long>(this, unary_at, "assign double words", long(0));
}

void CHexEditView::OnAssign64bit() 
{
    ::OnOperateUnary<__int64>(this, unary_at, "assign quad words", __int64(0));
}

void CHexEditView::OnSeedCalc() 
{
    unsigned long seed = unsigned long(((CMainFrame *)AfxGetMainWnd())->m_wndCalc.GetValue());
    srand(seed);                        // Seed compiler PRNG (simple one)
    rand_good_seed(seed);               // Seed our own PRNG
    theApp.SaveToMacro(km_seed, 1);
}

void CHexEditView::OnSeedRandom() 
{
    unsigned long seed = ::GetTickCount();
    srand(seed);                        // Seed compiler PRNG (simple fast one)
    rand_good_seed(seed);               // Seed our own PRNG
    theApp.SaveToMacro(km_seed);
}

void CHexEditView::OnRandByte() 
{
    ::OnOperateUnary<char>(this, unary_rand, "randomize bytes", char(0));
}

void CHexEditView::OnRandFast() 
{
    ::OnOperateUnary<char>(this, unary_rand_fast, "fast randomize bytes", char(0));
}

// This is connected to Ctrl+T and is used for testing new dialogs etc
void CHexEditView::OnViewtest() 
{
	// move this to a command xxx
	OnMd5();
}

CTipExpr::value_t CTipExpr::find_symbol(const char *sym, value_t parent, size_t index, int *pac,
        __int64 &sym_size, __int64 &sym_address)
{
    ASSERT(parent.typ == TYPE_NONE);    // Parent is not used here
    value_t retval;
    retval.typ = TYPE_NONE;             // Default to symbol not found
    retval.error = false;
    retval.int64 = 0;
    sym_address = pview_->tip_addr_;
	unsigned_ = false;

	CString sym_str(sym);
	if (sym_str.CompareNoCase("address") == 0)
	{
		retval.typ = TYPE_INT;          // Just return the address
		retval.int64 = sym_address;
		sym_size = 4;                   // Addresses can be up to 8 but this restricts display to 32-bits (and if bigger then extra bits are shown)
		unsigned_ = true;
	}
	else if (sym_str.CompareNoCase("sector") == 0)
	{
		retval.typ = TYPE_INT;          // current sector
		retval.int64 = sym_address / pview_->GetDocument()->GetSectorSize();
		sym_size = 4;
		unsigned_ = true;
	}
	else if (sym_str.CompareNoCase("offset") == 0)
	{
		retval.typ = TYPE_INT;          // offset within current sector
		retval.int64 = sym_address % pview_->GetDocument()->GetSectorSize();
		sym_size = 4;
		unsigned_ = true;
	}
	else if (sym_str.CompareNoCase("sbyte") == 0)
	{
        signed char val;
        if (pview_->GetDocument()->GetData((unsigned char *)&val, sizeof(val), sym_address))
		{
			retval.typ = TYPE_INT;          // Just return the current byte
			retval.int64 = val;
			sym_size = sizeof(val);
		}
	}
	else if (sym_str.CompareNoCase("byte") == 0)
	{
        unsigned char val;
        if (pview_->GetDocument()->GetData(&val, sizeof(val), sym_address))
		{
			retval.typ = TYPE_INT;          // Just return the current byte
			retval.int64 = val;
			sym_size = sizeof(val);
			unsigned_ = true;
		}
	}
	else if (sym_str.CompareNoCase("word") == 0)
	{
        short val;
        if (pview_->GetDocument()->GetData((unsigned char *)&val, sizeof(val), sym_address))
		{
			if (pview_->BigEndian())
				flip_bytes((unsigned char *)&val, sizeof(val));
			retval.typ = TYPE_INT;
			retval.int64 = val;
			sym_size = sizeof(val);
		}
	}
	else if (sym_str.CompareNoCase("uword") == 0)
	{
        unsigned short val;
        if (pview_->GetDocument()->GetData((unsigned char *)&val, sizeof(val), sym_address))
		{
			if (pview_->BigEndian())
				flip_bytes((unsigned char *)&val, sizeof(val));
			retval.typ = TYPE_INT;
			retval.int64 = val;
			sym_size = sizeof(val);
			unsigned_ = true;
		}
	}
	else if (sym_str.CompareNoCase("dword") == 0)
	{
        long val;
        if (pview_->GetDocument()->GetData((unsigned char *)&val, sizeof(val), sym_address))
		{
			if (pview_->BigEndian())
				flip_bytes((unsigned char *)&val, sizeof(val));
			retval.typ = TYPE_INT;
			retval.int64 = val;
			sym_size = sizeof(val);
		}
	}
	else if (sym_str.CompareNoCase("udword") == 0)
	{
        unsigned long val;
        if (pview_->GetDocument()->GetData((unsigned char *)&val, sizeof(val), sym_address))
		{
			if (pview_->BigEndian())
				flip_bytes((unsigned char *)&val, sizeof(val));
			retval.typ = TYPE_INT;
			retval.int64 = val;
			sym_size = sizeof(val);
			unsigned_ = true;
		}
	}
	else if (sym_str.CompareNoCase("qword") == 0)
	{
        __int64 val;
        if (pview_->GetDocument()->GetData((unsigned char *)&val, sizeof(val), sym_address))
		{
			if (pview_->BigEndian())
				flip_bytes((unsigned char *)&val, sizeof(val));
			retval.typ = TYPE_INT;
			retval.int64 = val;
			sym_size = sizeof(val);
		}
	}
	else if (sym_str.CompareNoCase("uqword") == 0)
	{
        unsigned __int64 val;
        if (pview_->GetDocument()->GetData((unsigned char *)&val, sizeof(val), sym_address))
		{
			if (pview_->BigEndian())
				flip_bytes((unsigned char *)&val, sizeof(val));
			retval.typ = TYPE_INT;
			retval.int64 = val;
			sym_size = sizeof(val);
			unsigned_ = true;
		}
	}
	else if (sym_str.CompareNoCase("ieee32") == 0)
	{
        float val;
        if (pview_->GetDocument()->GetData((unsigned char *)&val, sizeof(val), sym_address))
		{
			if (pview_->BigEndian())
				flip_bytes((unsigned char *)&val, sizeof(val));
			retval.typ = TYPE_REAL;
			retval.real64 = val;
			sym_size = sizeof(val);
		}
	}
	else if (sym_str.CompareNoCase("ieee64") == 0)
	{
        double val;
        if (pview_->GetDocument()->GetData((unsigned char *)&val, sizeof(val), sym_address))
		{
			if (pview_->BigEndian())
				flip_bytes((unsigned char *)&val, sizeof(val));
			retval.typ = TYPE_REAL;
			retval.real64 = val;
			sym_size = sizeof(val);
		}
	}
	else if (sym_str.CompareNoCase("ibm32") == 0)
	{
        unsigned char val[4];
        if (pview_->GetDocument()->GetData(val, sizeof(val), sym_address))
		{
			retval.typ = TYPE_REAL;
			retval.real64 = ibm_fp32(val, NULL, NULL, !pview_->BigEndian());
			sym_size = sizeof(val);
		}
	}
	else if (sym_str.CompareNoCase("ibm64") == 0)
	{
        unsigned char val[8];
        if (pview_->GetDocument()->GetData(val, sizeof(val), sym_address))
		{
			retval.typ = TYPE_REAL;
			retval.real64 = ibm_fp64(val, NULL, NULL, !pview_->BigEndian());
			sym_size = sizeof(val);
		}
	}

	size_ = int(sym_size);  // Remember size of the symbol we encountered
	return retval;
}