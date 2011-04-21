#include "fitz.h"
#include "mupdf.h"

#ifdef NOCJK
#define NOCJKFONT
#endif

#include "../generated/font_base14.h"

#ifndef NODROIDFONT
#include "../generated/font_droid.h"
#endif

#ifndef NOCJKFONT
#include "../generated/font_cjk.h"
#endif

unsigned char *
pdf_find_builtin_font(char *name, unsigned int *len)
{
	if (!strcmp("Courier", name)) {
		*len = sizeof pdf_font_NimbusMonL_Regu;
		return (unsigned char*) pdf_font_NimbusMonL_Regu;
	}
	if (!strcmp("Courier-Bold", name)) {
		*len = sizeof pdf_font_NimbusMonL_Bold;
		return (unsigned char*) pdf_font_NimbusMonL_Bold;
	}
	if (!strcmp("Courier", name)) {
		*len = sizeof pdf_font_NimbusMonL_Regu;
		return (unsigned char*) pdf_font_NimbusMonL_Regu;
	}
	if (!strcmp("Courier-Bold", name)) {
		*len = sizeof pdf_font_NimbusMonL_Bold;
		return (unsigned char*) pdf_font_NimbusMonL_Bold;
	}
	if (!strcmp("Courier-Oblique", name)) {
		*len = sizeof pdf_font_NimbusMonL_ReguObli;
		return (unsigned char*) pdf_font_NimbusMonL_ReguObli;
	}
	if (!strcmp("Courier-BoldOblique", name)) {
		*len = sizeof pdf_font_NimbusMonL_BoldObli;
		return (unsigned char*) pdf_font_NimbusMonL_BoldObli;
	}
	if (!strcmp("Helvetica", name)) {
		*len = sizeof pdf_font_NimbusSanL_Regu;
		return (unsigned char*) pdf_font_NimbusSanL_Regu;
	}
	if (!strcmp("Helvetica-Bold", name)) {
		*len = sizeof pdf_font_NimbusSanL_Bold;
		return (unsigned char*) pdf_font_NimbusSanL_Bold;
	}
	if (!strcmp("Helvetica-Oblique", name)) {
		*len = sizeof pdf_font_NimbusSanL_ReguItal;
		return (unsigned char*) pdf_font_NimbusSanL_ReguItal;
	}
	if (!strcmp("Helvetica-BoldOblique", name)) {
		*len = sizeof pdf_font_NimbusSanL_BoldItal;
		return (unsigned char*) pdf_font_NimbusSanL_BoldItal;
	}
	if (!strcmp("Times-Roman", name)) {
		*len = sizeof pdf_font_NimbusRomNo9L_Regu;
		return (unsigned char*) pdf_font_NimbusRomNo9L_Regu;
	}
	if (!strcmp("Times-Bold", name)) {
		*len = sizeof pdf_font_NimbusRomNo9L_Medi;
		return (unsigned char*) pdf_font_NimbusRomNo9L_Medi;
	}
	if (!strcmp("Times-Italic", name)) {
		*len = sizeof pdf_font_NimbusRomNo9L_ReguItal;
		return (unsigned char*) pdf_font_NimbusRomNo9L_ReguItal;
	}
	if (!strcmp("Times-BoldItalic", name)) {
		*len = sizeof pdf_font_NimbusRomNo9L_MediItal;
		return (unsigned char*) pdf_font_NimbusRomNo9L_MediItal;
	}
	if (!strcmp("Symbol", name)) {
		*len = sizeof pdf_font_StandardSymL;
		return (unsigned char*) pdf_font_StandardSymL;
	}
	if (!strcmp("ZapfDingbats", name)) {
		*len = sizeof pdf_font_Dingbats;
		return (unsigned char*) pdf_font_Dingbats;
	}
	*len = 0;
	return NULL;
}

unsigned char *
pdf_find_substitute_font(int mono, int serif, int bold, int italic, unsigned int *len)
{
#ifdef NODROIDFONT
	if (mono) {
		if (bold) {
			if (italic) return pdf_find_builtin_font("Courier-BoldOblique", len);
			else return pdf_find_builtin_font("Courier-Bold", len);
		} else {
			if (italic) return pdf_find_builtin_font("Courier-Oblique", len);
			else return pdf_find_builtin_font("Courier", len);
		}
	} else if (serif) {
		if (bold) {
			if (italic) return pdf_find_builtin_font("Times-BoldItalic", len);
			else return pdf_find_builtin_font("Times-Bold", len);
		} else {
			if (italic) return pdf_find_builtin_font("Times-Italic", len);
			else return pdf_find_builtin_font("Times-Roman", len);
		}
	} else {
		if (bold) {
			if (italic) return pdf_find_builtin_font("Helvetica-BoldOblique", len);
			else return pdf_find_builtin_font("Helvetica-Bold", len);
		} else {
			if (italic) return pdf_find_builtin_font("Helvetica-Oblique", len);
			else return pdf_find_builtin_font("Helvetica", len);
		}
	}
#else
	*len = sizeof pdf_font_DroidSans;
	return (unsigned char*) pdf_font_DroidSans;
#endif
}

unsigned char *
pdf_find_substitute_cjk_font(int ros, int serif, unsigned int *len)
{
#ifndef NOCJKFONT
	*len = sizeof pdf_font_DroidSansFallback;
	return (unsigned char*) pdf_font_DroidSansFallback;
#else
	*len = 0;
	return NULL;
#endif
}

/* SumatraPDF: also load fonts included with Windows */
#ifdef _WIN32

#include <windows.h>
#include <tchar.h>

// TODO: Use more of FreeType for TTF parsing (for performance reasons,
//       the fonts can't be parsed completely, though)
#include <ft2build.h>
#include FT_TRUETYPE_IDS_H
#include FT_TRUETYPE_TAGS_H

#define SWAPWORD(x)		MAKEWORD(HIBYTE(x), LOBYTE(x))
#define SWAPLONG(x)		MAKELONG(SWAPWORD(HIWORD(x)), SWAPWORD(LOWORD(x)))

#define TTC_VERSION1	0x00010000
#define TTC_VERSION2	0x00020000

#define MAX_FACENAME	128

// Note: the font face must be the first field so that the structure
//       can be treated like a simple string for searching
typedef struct pdf_fontmapMS_s
{
	char fontface[MAX_FACENAME]; // UTF-8 encoded
	char fontpath[MAX_PATH];     // ANSI encoded
	int index;
} pdf_fontmapMS;

typedef struct pdf_fontlistMS_s
{
	pdf_fontmapMS *fontmap;
	int len;
	int cap;
} pdf_fontlistMS;

typedef struct _tagTT_OFFSET_TABLE
{
	ULONG	uVersion;
	USHORT	uNumOfTables;
	USHORT	uSearchRange;
	USHORT	uEntrySelector;
	USHORT	uRangeShift;
} TT_OFFSET_TABLE;

typedef struct _tagTT_TABLE_DIRECTORY
{
	ULONG	uTag;				//table name
	ULONG	uCheckSum;			//Check sum
	ULONG	uOffset;			//Offset from beginning of file
	ULONG	uLength;			//length of the table in bytes
} TT_TABLE_DIRECTORY;

typedef struct _tagTT_NAME_TABLE_HEADER
{
	USHORT	uFSelector;			//format selector. Always 0
	USHORT	uNRCount;			//Name Records count
	USHORT	uStorageOffset;		//Offset for strings storage, from start of the table
} TT_NAME_TABLE_HEADER;

typedef struct _tagTT_NAME_RECORD
{
	USHORT	uPlatformID;
	USHORT	uEncodingID;
	USHORT	uLanguageID;
	USHORT	uNameID;
	USHORT	uStringLength;
	USHORT	uStringOffset;	//from start of storage area
} TT_NAME_RECORD;

typedef struct _tagFONT_COLLECTION
{
	ULONG	Tag;
	ULONG	Version;
	ULONG	NumFonts;
} FONT_COLLECTION;

static struct {
	char *name;
	char *pattern;
} baseSubstitutes[] = {
	{ "Courier", "CourierNewPSMT" },
	{ "Courier-Bold", "CourierNewPS-BoldMT" },
	{ "Courier-Oblique", "CourierNewPS-ItalicMT" },
	{ "Courier-BoldOblique", "CourierNewPS-BoldItalicMT" },
	{ "Helvetica", "ArialMT" },
	{ "Helvetica-Bold", "Arial-BoldMT" },
	{ "Helvetica-Oblique", "Arial-ItalicMT" },
	{ "Helvetica-BoldOblique", "Arial-BoldItalicMT" },
	{ "Times-Roman", "TimesNewRomanPSMT" },
	{ "Times-Bold", "TimesNewRomanPS-BoldMT" },
	{ "Times-Italic", "TimesNewRomanPS-ItalicMT" },
	{ "Times-BoldItalic", "TimesNewRomanPS-BoldItalicMT" },
	{ "Symbol", "SymbolMT" },
};

static pdf_fontlistMS fontlistMS =
{
	NULL,
	0,
	0,
};

/* A little bit more sophisticated name matching so that e.g. "EurostileExtended"
   matches "EurostileExtended-Roman" or "Tahoma-Bold,Bold" matches "Tahoma-Bold" */
static int
lookupcompare(const void *elem1, const void *elem2)
{
	char *val1 = (char *)elem1;
	char *val2 = (char *)elem2;
	int len1 = strlen(val1);
	int len2 = strlen(val2);

	if (len1 != len2)
	{
		char *rest = len1 > len2 ? val1 + len2 : val2 + len1;
		if (',' == *rest || !_stricmp(rest, "-roman"))
			return _strnicmp(val1, val2, MIN(len1, len2));
	}

	return _stricmp(val1, val2);
}

static void
removespaces(char *srcDest)
{
	char *dest;

	for (dest = srcDest; *srcDest; srcDest++)
		if (*srcDest != ' ')
			*dest++ = *srcDest;
	*dest = 0;
}

/* source and dest can be same */
static fz_error
decodeunicodeBMP(char* source, int sourcelen, char* dest, int destlen)
{
	wchar_t tmp[1024 * 2];
	int converted, i;

	if (sourcelen % 2 != 0)
		return fz_throw("fonterror : invalid unicode string");

	memset(tmp, 0, sizeof(tmp));
	for (i = 0; i < sourcelen / 2; i++)
		tmp[i] = SWAPWORD(((wchar_t *)source)[i]);

	converted = WideCharToMultiByte(CP_UTF8, 0, tmp, -1, dest, destlen, NULL, NULL);
	if (converted == 0)
		return fz_throw("fonterror : invalid unicode string");

	return fz_okay;
}

static fz_error
decodeplatformstring(int platform, int enctype, char* source, int sourcelen, char* dest, int destlen)
{
	switch (platform)
	{
	case TT_PLATFORM_APPLE_UNICODE:
		switch (enctype)
		{
		case TT_APPLE_ID_DEFAULT:
		case TT_APPLE_ID_UNICODE_2_0:
			return decodeunicodeBMP(source, sourcelen, dest, destlen);
		}
		return fz_throw("fonterror : unsupported encoding");
	case TT_PLATFORM_MACINTOSH:
		switch( enctype)
		{
		case TT_MAC_ID_ROMAN:
			if (sourcelen + 1 > destlen)
				return fz_throw("fonterror : short buf length");
			// TODO: Convert to UTF-8 from what encoding?
			memcpy(dest, source, sourcelen);
			dest[sourcelen] = 0;
			return fz_okay;
		}
		return fz_throw("fonterror : unsupported encoding");
	case TT_PLATFORM_MICROSOFT:
		switch (enctype)
		{
		case TT_MS_ID_SYMBOL_CS:
		case TT_MS_ID_UNICODE_CS:
		case TT_MS_ID_UCS_4:
			return decodeunicodeBMP(source, sourcelen, dest, destlen);
		}
		return fz_throw("fonterror : unsupported encoding");
	default:
		return fz_throw("fonterror : unsupported platform");
	}
}

static fz_error
growfontlist(pdf_fontlistMS *fl)
{
	int newcap;
	pdf_fontmapMS *newitems;

	if (fl->cap == 0)
		newcap = 1024;
	else
		newcap = fl->cap * 2;

	newitems = fz_realloc(fl->fontmap, newcap, sizeof(pdf_fontmapMS));
	if (!newitems)
		return fz_rethrow(-1, "out of memory");;

	memset(newitems + fl->cap, 0, sizeof(pdf_fontmapMS) * (newcap - fl->cap));

	fl->fontmap = newitems;
	fl->cap = newcap;

	return fz_okay;
}

static fz_error
insertmapping(pdf_fontlistMS *fl, char *facename, char *path, int index)
{
	if (fl->len == fl->cap)
	{
		fz_error err = growfontlist(fl);
		if (err) return err;
	}

	if (fl->len >= fl->cap)
		return fz_throw("fonterror : fontlist overflow");

	fz_strlcpy(fl->fontmap[fl->len].fontface, facename, sizeof(fl->fontmap[0].fontface));
	fz_strlcpy(fl->fontmap[fl->len].fontpath, path, sizeof(fl->fontmap[0].fontpath));
	fl->fontmap[fl->len].index = index;

	++fl->len;

	return fz_okay;
}

static fz_error
safe_read(fz_stream *file, char *buf, int size)
{
	int n = fz_read(file, buf, size);
	if (n != size) /* covers n < 0 case */
		return fz_throw("ioerror");

	return fz_okay;
}

static fz_error
read_ttf_string(fz_stream *file, int offset, TT_NAME_RECORD *ttRecord, char *buf, int size)
{
	fz_error err;
	char szTemp[MAX_FACENAME * 2];
	// ignore empty and overlong strings
	int stringLength = SWAPWORD(ttRecord->uStringLength);
	if (stringLength == 0 || stringLength >= sizeof(szTemp))
		return fz_okay;

	fz_seek(file, offset + SWAPWORD(ttRecord->uStringOffset), 0);
	err = safe_read(file, szTemp, stringLength);
	if (err) return err;
	return decodeplatformstring(SWAPWORD(ttRecord->uPlatformID), SWAPWORD(ttRecord->uEncodingID),
		szTemp, stringLength, buf, size);
}

static fz_error
parseTTF(fz_stream *file, int offset, int index, char *path)
{
	fz_error err = fz_okay;

	TT_OFFSET_TABLE ttOffsetTable;
	TT_TABLE_DIRECTORY tblDir;
	TT_NAME_TABLE_HEADER ttNTHeader;
	TT_NAME_RECORD ttRecord;

	char szPSName[MAX_FACENAME] = { 0 }, szTTName[MAX_FACENAME] = { 0 }, szStyle[MAX_FACENAME] = { 0 };
	int i, count, tblOffset;

	fz_seek(file,offset,0);
	err = safe_read(file, (char *)&ttOffsetTable, sizeof(TT_OFFSET_TABLE));
	if (err) return err;

	// check if this is a TrueType font of version 1.0 or an OpenType font
	if (SWAPLONG(ttOffsetTable.uVersion) != TTC_VERSION1 && ttOffsetTable.uVersion != TTAG_OTTO)
	{
		return fz_throw("fonterror : invalid font version");
	}

	// determine the name table's offset by iterating through the offset table
	count = SWAPWORD(ttOffsetTable.uNumOfTables);
	for (i = 0; i < count; i++)
	{
		err = safe_read(file, (char *)&tblDir, sizeof(TT_TABLE_DIRECTORY));
		if (err) return err;
		if (!tblDir.uTag || SWAPLONG(tblDir.uTag) == TTAG_name)
			break;
	}
	if (count == i || !tblDir.uTag)
		return fz_throw("fonterror : nameless font");
	tblOffset = SWAPLONG(tblDir.uOffset);

	// read the 'name' table for record count and offsets
	fz_seek(file, tblOffset, 0);
	err = safe_read(file, (char *)&ttNTHeader, sizeof(TT_NAME_TABLE_HEADER));
	if (err) return err;
	offset = tblOffset + sizeof(TT_NAME_TABLE_HEADER);
	tblOffset += SWAPWORD(ttNTHeader.uStorageOffset);

	// read through the strings for PostScript name and font family
	count = SWAPWORD(ttNTHeader.uNRCount);
	for (i = 0; i < count; i++)
	{
		short nameId;

		fz_seek(file, offset + i * sizeof(TT_NAME_RECORD), 0);
		err = safe_read(file, (char *)&ttRecord, sizeof(TT_NAME_RECORD));
		if (err) return err;

		// ignore non-English strings
		if (ttRecord.uLanguageID && SWAPWORD(ttRecord.uLanguageID) != TT_MS_LANGID_ENGLISH_UNITED_STATES)
			continue;
		// ignore names other than font (sub)family and PostScript name
		nameId = SWAPWORD(ttRecord.uNameID);
		if (TT_NAME_ID_FONT_FAMILY == nameId)
			err = read_ttf_string(file, tblOffset, &ttRecord, szTTName, MAX_FACENAME);
		else if (TT_NAME_ID_FONT_SUBFAMILY == nameId)
			err = read_ttf_string(file, tblOffset, &ttRecord, szStyle, MAX_FACENAME);
		else if (TT_NAME_ID_PS_NAME == nameId)
			err = read_ttf_string(file, tblOffset, &ttRecord, szPSName, MAX_FACENAME);
		if (err) fz_catch(err, "ignoring face name decoding fonterror");
	}

	// TODO: is there a better way to distinguish Arial Caps from Arial proper?
	// cf. http://code.google.com/p/sumatrapdf/issues/detail?id=1290
	if (!strcmp(szPSName, "ArialMT") && (strstr(path, "caps") || strstr(path, "Caps")))
		return fz_throw("ignore %s, as it can't be distinguished from Arial,Regular", path);

	if (szPSName[0])
	{
		err = insertmapping(&fontlistMS, szPSName, path, index);
		if (err) return err;
	}
	if (szTTName[0])
	{
		// derive a PostScript-like name and add it, if it's different from the font's
		// included PostScript name; cf. http://code.google.com/p/sumatrapdf/issues/detail?id=376

		// append the font's subfamily, unless it's a Regular font
		if (szStyle[0] && _stricmp(szStyle, "Regular") != 0)
		{
			fz_strlcat(szTTName, "-", MAX_FACENAME);
			fz_strlcat(szTTName, szStyle, MAX_FACENAME);
		}
		removespaces(szTTName);
		// compare the two names before adding this one
		if (lookupcompare(szTTName, szPSName))
		{
			err = insertmapping(&fontlistMS, szTTName, path, index);
			if (err) return err;
		}
	}
	return fz_okay;
}

static fz_error
parseTTFs(char *path)
{
	fz_error err;
	fz_stream *file = fz_open_file(path);
	if (!file)
		return fz_throw("fonterror : %s not found", path);

	err = parseTTF(file, 0, 0, path);
	fz_close(file);
	return err;
}

static fz_error
parseTTCs(char *path)
{
	FONT_COLLECTION fontcollection;
	ULONG i, numFonts, *offsettable = NULL;
	fz_error err;

	fz_stream *file = fz_open_file(path);
	if (!file)
	{
		err = fz_throw("fonterror : %s not found", path);
		goto cleanup;
	}

	err = safe_read(file, (char *)&fontcollection, sizeof(FONT_COLLECTION));
	if (err) goto cleanup;
	if (SWAPLONG(fontcollection.Tag) != TTAG_ttcf)
	{
		err = fz_throw("fonterror : wrong format");
		goto cleanup;
	}

	if (SWAPLONG(fontcollection.Version) != TTC_VERSION1 && SWAPLONG(fontcollection.Version) != TTC_VERSION2)
	{
		err = fz_throw("fonterror : invalid version");
		goto cleanup;
	}

	numFonts = SWAPLONG(fontcollection.NumFonts);
	offsettable = fz_calloc(numFonts, sizeof(ULONG));
	if (offsettable == NULL)
	{
		err = fz_throw("out of memory");
		goto cleanup;
	}

	err = safe_read(file, (char *)offsettable, numFonts * sizeof(ULONG));
	for (i = 0; i < numFonts && !err; i++)
	{
		err = parseTTF(file, SWAPLONG(offsettable[i]), i, path);
	}

cleanup:
	if (offsettable)
		fz_free(offsettable);
	if (file)
		fz_close(file);

	return err;
}

static fz_error
pdf_createfontlistMS()
{
	TCHAR szFontDir[MAX_PATH], szFile[MAX_PATH];
	char szPathAnsi[MAX_PATH], *fileExt;
	HANDLE hList;
	WIN32_FIND_DATA FileData;

	// Get the proper directory path
	GetWindowsDirectory(szFontDir, _countof(szFontDir));
	_tcscat_s(szFontDir, MAX_PATH, _T("\\Fonts\\*.?t?"));

	hList = FindFirstFile(szFontDir, &FileData);
	if (hList == INVALID_HANDLE_VALUE)
	{
		/* Don't complain about missing directories */
		if (errno == ENOENT)
			return fz_throw("fonterror : can't find system fonts dir");
		return fz_throw("ioerror");
	}
	// drop the wildcards
	szFontDir[lstrlen(szFontDir) - 5] = 0;
	// Traverse through the directory structure
	do
	{
		if (!(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			BOOL isNonAnsiPath = FALSE;
			// Get the full path for sub directory
			_sntprintf(szFile, MAX_PATH, _T("%s%s"), szFontDir, FileData.cFileName);
			szFile[MAX_PATH - 1] = '\0';
#ifdef _UNICODE
			// FreeType uses fopen and thus requires the path to be in the ANSI code page
			WideCharToMultiByte(CP_ACP, 0, szFile, -1, szPathAnsi, sizeof(szPathAnsi), NULL, &isNonAnsiPath);
#else
			strcpy(szPathAnsi, szFile);
			isNonAnsiPath = strchr(szPathAnsi, '?') != NULL;
#endif
			fileExt = szPathAnsi + strlen(szPathAnsi) - 4;
			if (isNonAnsiPath)
				fz_warn("ignoring font with non-ANSI filename: %s", szFile);
			else if (!_stricmp(fileExt, ".ttc"))
				parseTTCs(szPathAnsi);
			else if (!_stricmp(fileExt, ".ttf") || !_stricmp(fileExt, ".otf"))
				parseTTFs(szPathAnsi);
			// ignore errors occurring while parsing a given font file
		}
	} while (FindNextFile(hList, &FileData));
	FindClose(hList);

#ifdef NOCJKFONT
	{
		// If no CJK fallback font is builtin but one has been shipped separately (in the same
		// directory as the main executable), add it to the list of loadable system fonts
		TCHAR *lpFileName;
		HANDLE hFile;

		GetModuleFileName(0, szFontDir, MAX_PATH);
		GetFullPathName(szFontDir, MAX_PATH, szFile, &lpFileName);
		lstrcpyn(lpFileName, _T("DroidSansFallback.ttf"), MAX_PATH - (lpFileName - szFile));

		hFile = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			BOOL isNonAnsiPath = FALSE;
#ifdef _UNICODE
			WideCharToMultiByte(CP_ACP, 0, szFile, -1, szPathAnsi, sizeof(szPathAnsi), NULL, &isNonAnsiPath);
#else
			strcpy(szPathAnsi, szFile);
			isNonAnsiPath = strchr(szPathAnsi, '?') != NULL;
#endif
			if (!isNonAnsiPath)
				insertmapping(&fontlistMS, "DroidSansFallback", szPathAnsi, 0);
			else
				fz_warn("ignoring font with non-ANSI filename: %s", szPathAnsi);
			CloseHandle(hFile);
		}
	}
#endif

	// sort the font list, so that it can be searched binarily
	qsort(fontlistMS.fontmap, fontlistMS.len, sizeof(pdf_fontmapMS), _stricmp);
	// TODO: make "TimesNewRomanPSMT" default substitute font?

	return fz_okay;
}

static int
pdf_destroyfontlistMS(void)
{
	if (fontlistMS.fontmap != NULL)
		fz_free(fontlistMS.fontmap);

	fontlistMS.fontmap = NULL;
	fontlistMS.len = 0;
	fontlistMS.cap = 0;

	return 0;
}

fz_error
pdf_load_windows_font(pdf_font_desc *font, char *fontname)
{
	fz_error error;
	pdf_fontmapMS *found = NULL;
	char *comma;

	if (fontlistMS.len == 0)
	{
		pdf_createfontlistMS();
		if (fontlistMS.len == 0)
			return fz_throw("fonterror : no fonts in the system");
		_onexit(pdf_destroyfontlistMS);
	}

	if (getenv("MULOG"))
		printf("pdf_load_windows_font: looking for font '%s'\n", fontname);

	// work on a normalized copy of the font name
	fontname = strdup(fontname);
	removespaces(fontname);

	// first, try to find the exact font name (including appended style information)
	comma = strchr(fontname, ',');
	if (comma)
	{
		*comma = '-';
		found = bsearch(fontname, fontlistMS.fontmap, fontlistMS.len, sizeof(pdf_fontmapMS), lookupcompare);
		*comma = ',';
	}
	// second, substitute the font name with a known PostScript name
	if (!found)
	{
		int i;
		for (i = 0; i < _countof(baseSubstitutes); i++)
			if (!strcmp(fontname, baseSubstitutes[i].name))
				break;
		if (i < _countof(baseSubstitutes))
			found = bsearch(baseSubstitutes[i].pattern, fontlistMS.fontmap, fontlistMS.len, sizeof(pdf_fontmapMS), lookupcompare);
	}
	// third, search for the font name without additional style information
	if (!found)
		found = bsearch(fontname, fontlistMS.fontmap, fontlistMS.len, sizeof(pdf_fontmapMS), lookupcompare);
	free(fontname);

	if (!found)
		return !fz_okay;

	// TODO: use fz_new_font_from_buffer so that fontpath can be a proper TCHAR[]
	error = fz_new_font_from_file(&font->font, found->fontpath, found->index);
	if (error)
		return fz_rethrow(error, "cannot load freetype font from a file %s", found->fontpath);

	font->font->ft_file = fz_strdup(found->fontpath);

	if (getenv("MULOG"))
		printf("pdf_load_windows_font: loading font from '%s'\n", found->fontpath);

	return fz_okay;
}

fz_error
pdf_load_similar_cjk_font(pdf_font_desc *font, int ros, int serif)
{
	if (!serif)
	{
		switch (ros)
		{
		case PDF_ROS_CNS: return pdf_load_windows_font(font, "MingLiU");
		case PDF_ROS_GB: return pdf_load_windows_font(font, "SimSun");
		case PDF_ROS_JAPAN: return pdf_load_windows_font(font, "MS-Mincho");
		case PDF_ROS_KOREA: return pdf_load_windows_font(font, "Batang");
		}
	}
	else
	{
		switch (ros)
		{
		case PDF_ROS_CNS: return pdf_load_windows_font(font, "DFKaiShu-SB-Estd-BF");
		case PDF_ROS_GB:
			if (fz_okay == pdf_load_windows_font(font, "KaiTi"))
				return fz_okay;
			return pdf_load_windows_font(font, "KaiTi_GB2312");
		case PDF_ROS_JAPAN: return pdf_load_windows_font(font, "MS-Gothic");
		case PDF_ROS_KOREA: return pdf_load_windows_font(font, "Gulim");
		}
	}
	return -1;
}

#endif
