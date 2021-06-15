// eqmacro.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "eqmacro.h"
#include <GnuArg.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

using namespace std;

#define SOCIAL_FILE_OFFSET			   0x019c
#define SOCIAL_STRUCT_SIZE			   0x0136
#define SOCIAL_PAGES						10
#define SOCIAL_BUTTONS_PER_PAGE		12
#define SOCIAL_MAX_LINES				5
#define SOCIAL_LINE_SIZE				60
#define SOCIAL_NAME_SIZE				9

#define HOTBUTTON_FILE_OFFSET			0x0c
#define HOTBUTTON_STRUCT_SIZE			0x04
#define HOTBUTTON_PAGES				   10
#define HOTBUTTONS_PER_PAGE			10

class CSoc // Social
	{
public:
	char pszName[SOCIAL_NAME_SIZE];
	char pszLine[SOCIAL_MAX_LINES][SOCIAL_LINE_SIZE];
	char cColor; // ???
	};


class CHB // HotButton
	{
public:
	WORD iType;
	WORD iIndex;
	};

static CSoc cSocials[SOCIAL_PAGES][SOCIAL_BUTTONS_PER_PAGE];

static CHB cHotButtons [HOTBUTTON_PAGES][HOTBUTTONS_PER_PAGE];

static PSZ pszInFile;
static int iFILELINE = 0;

BOOL bSOCIALS_ONLY;
BOOL bHOTBUTTONS_ONLY;
BOOL bNO_COMMENTS;


PSZ ppszTypes[] =
	{
	"",				  // 0
	"MainCombat",	  // 1
	"ClassCombat",	  // 2
	"Ability",	 	  // 3
	"Social",		  // 4
	"Inventory",	  // 5
	"Main",			  // 6
	"Spell",			  // 7
	NULL
	};

PSZ ppszMainCombat[] =
	{
	"Melee",
	"Range",
	NULL
	};

PSZ ppszInventory[] =	
	{
	"Pos0",
	"Pos1",
	"Pos2",
	"Pos3",
	"Pos4",
	"Pos5",
	"Pos6",
	"Pos7",
	"Pos8",
	"Pos9",
	"Posa",
	"Posb",
	"Posc",
	"Posd",
	"Pose",
	"Posf",
	"Pos10",
	"PosChest",
	"Pos12",
	"Pos13",
	"Pos14",
	"Pos15",
	"PosGeneral1",
	"PosGeneral2",
	"PosGeneral3",
	"PosGeneral4",
	"PosGeneral5",
	"PosGeneral6",
	"PosGeneral7",
	"PosGeneral8",
	"Pos17",
	NULL
	};

PSZ ppszMain[]	=
	{
	"Who",		  // 0
	"Invite",	  // 1
	"Disband",	  // 2
	"Camp",		  // 3
	"Sit",		  // 4
	"Walk",		  // 5
	"Help",		  // 6
	"Options",	  // 7
	"View",		  // 8
	"Persona",	  // 9
	"Spells",     // a
	NULL			  //
	};					  



/////////////////////////////////////////////////////////////////////////////
//
//	Util fns
//
/////////////////////////////////////////////////////////////////////////////


int Usage ()
	{
	printf ("EQMACRO Extracts and Sets EQ socials and Hot Buttons from opt files\n");
	printf ("USAGE: EQMACRO infilename [outfilename]\n");
	printf ("WHERE: infilename is the input file (a .opt or a .txt file)\n");
	printf ("\n");
	printf ("DESCRIPTION:\n");
	printf ("	This program is used to do 2 things:\n");
	printf ("	1> Extract an EverQuest character's socials macro's to a text file\n");
	printf ("	2> Use a text file to set an Everquest character's Social Macro's\n");
	printf ("\n");
	printf ("EXAMPLES:\n");
	printf ("	EQMACRO Kleg_40.opt\n");
	printf ("	This command extracts the Socials macro's from Kleg_40.opt and writes \n");
	printf ("	them to a file named Kleg_40.txt\n");
	printf ("		\n");
	printf ("	EQMACRO Kleg_40.txt\n");
	printf ("	This command reads the Socials macro's from Kleg_40.txt and writes \n");
	printf ("	them to the named Kleg_40.opt file\n");
	printf ("\n");
	printf ("	EQMACRO Kleg_40.opt out.txt\n");
	printf ("	This command extracts the Socials macro's from Kleg_40.opt and writes \n");
	printf ("	them to a file named out.txt\n");
	exit (0);
	}

void GeneralComments (CStdioFile* pFile)
	{
	pFile->WriteString ("; Everquest Social Macro and HotButton dump file 1.0\n");
	pFile->WriteString ("; This file was originally created with EQMacro v1.0\n");
	pFile->WriteString (";\n");
	pFile->WriteString ("; This file contains a dump of all the Social Macros and \n");
	pFile->WriteString ("; All the Hot Button definitions from the following everquest\n");
	pFile->WriteString ("; chartacter opt file: ");
	pFile->WriteString (pszInFile);	pFile->WriteString ("\n");
	pFile->WriteString ("; You can add, delete and change information in this file\n");
	pFile->WriteString ("; and then use EQMacro to load these back into the opt file\n");
	}

void SocialMacroComments (CStdioFile* pFile)
	{
	pFile->WriteString ("\n\n");
	pFile->WriteString ("; Social Macro's\n");
	pFile->WriteString ("; --------------\n");
	pFile->WriteString ("; There is one section for each defined social macro.  The format\n");
	pFile->WriteString ("; of a social macro dump is:\n");
	pFile->WriteString (";\n");
	pFile->WriteString ("; [Social Page#.Button# Name]\n");
	pFile->WriteString ("; TextLine#1\n");
	pFile->WriteString ("; ...\n");
	pFile->WriteString ("; TextLine#5\n");
	pFile->WriteString (";\n");
	pFile->WriteString ("; Where:\n");
	pFile->WriteString (";  Page# is a number from (1 - 10) specifying the socials page where\n");
	pFile->WriteString (";   the macro is located.\n");
	pFile->WriteString (";  Button# is a number from (1 - 12) specifying the location of the \n");
	pFile->WriteString (";   social macro on the social page.  The button order is left to right,\n");
	pFile->WriteString (";   top to bottom.\n");
	pFile->WriteString (";  Name is the text to display on the Button (max length 8 chars)\n");
	pFile->WriteString (";  TextLine# is up to 5 lines of macro text (max length 59 chars)\n");
	pFile->WriteString (";\n");
	pFile->WriteString ("; You may add additional macro's as needed.  Make sure the Page/Button \n");
	pFile->WriteString ("; is unique if you add additional macro's\n");
	pFile->WriteString (";\n");
	pFile->WriteString ("; Also supported is the following Extended format:\n");
	pFile->WriteString ("; [XSocial Page#.Button# Name]\n");
	pFile->WriteString ("; OneLongTextLine\n");
	pFile->WriteString (";\n");
	pFile->WriteString ("; Where:\n");
	pFile->WriteString (";  OneLongTextLine is a single line of macro text (max length 299 chars)\n");
	pFile->WriteString (";  This is experimental, and you cannot properly edit/view the macro from EQ.\n");
	pFile->WriteString (";\n");
	}

void HotButtonComments (CStdioFile* pFile)
	{
	pFile->WriteString ("\n\n\n");
	pFile->WriteString ("; HotButtons\n");
	pFile->WriteString ("; ----------\n");
	pFile->WriteString ("; The [HotButtons] section defines all of the HotButtons\n");
	pFile->WriteString ("; There is one line for each button definition.  The definition\n");
	pFile->WriteString ("; format is as follows:\n");
	pFile->WriteString (";\n");
	pFile->WriteString ("; HotButtonPage#.HotButtonLocation# = Type.Instance\n");
	pFile->WriteString (";\n");
	pFile->WriteString ("; Where:\n");
	pFile->WriteString (";  HotButtonPage# is the HotButton page for the button (1 - 10)\n");
	pFile->WriteString (";  HotButtonLocation#  is the HotButton location for the button\n");
	pFile->WriteString (";   (1 - 10).  The button order is top to bottom, left to right.\n");
	pFile->WriteString (";  Type is the type of the macro button. The following Button \n");
	pFile->WriteString (";   types are supported:\n");
	pFile->WriteString (";		MainCombat	- Melee and Range combat  (1)\n");
	pFile->WriteString (";		ClassCombat	- Class specific combats  (2)\n");
	pFile->WriteString (";		Ability		- Abilities				  (3)\n");
	pFile->WriteString (";		Social		- Socials				  (4)\n");
	pFile->WriteString (";		Inventory	- Inventory Slots		  (5)\n");
	pFile->WriteString (";		Main		- Main screen Buttons	  (6)\n");
	pFile->WriteString (";		Spell		- Spell Buttons	  (8)\n");
	pFile->WriteString (";  Instance is a name or number that identifies the specific instance \n");
	pFile->WriteString (";  of the button. Valid Instance names depend on the Type:\n");
	pFile->WriteString (";    	Valid Instances for Type Main: Who, Invite, Disband, Camp, Sit, \n");
	pFile->WriteString (";    	  	Walk, Help, Options, View, Persona, Spells\n");
	pFile->WriteString (";	 	Valid Instances for MainCombat: Melee, Range\n");
	pFile->WriteString (";	 	Valid Instances for Inventory: PosChest, PosGeneral1 thru PosGeneral8\n");
	pFile->WriteString (";  	 	Valid Instances for Social: Any social name or page#.Button#\n");
	pFile->WriteString (";	For all other Types, the Instance must be specified via index.\n");
	pFile->WriteString (";\n");
	pFile->WriteString (";  For example these are all valid Type/Instance pairs:\n");
	pFile->WriteString (";	   Main.Help	  	Inventory.PosGeneral5   Social.Feedbk\n");
	pFile->WriteString (";	   Main.Persona	  	Inventory.PosChest	  	Social.Hail\n");
	pFile->WriteString (";	   MainCombat.Range	Ability.2	  			Social.2.3\n");
	pFile->WriteString (";\n");
	}


int Err (PSZ psz, ...)
	{
	va_list vlst;

	printf ("Error ");
	va_start (vlst, psz);
	vprintf (psz, vlst);
	va_end (vlst);
	printf ("\n");
	exit (1);
	return 0;
	}


static void PrintSocial (CSoc& CSoc, int iPage, int iButton)
	{
	printf ("[Page:%d Button:%d Name:%s]\n", iPage+1, iButton+1, CSoc.pszName);		
	for (int iLine=0; iLine<SOCIAL_MAX_LINES; iLine++)
		printf ("%s\n", CSoc.pszLine[iLine]);
	}


CString GetStr (CString strLine, CString strKeyWord, BOOL bQuoted = FALSE)
	{
	int iPos;

	if ((iPos = strLine.Find (strKeyWord)) == -1)
		Err ("Line %d: cannot find field %s in Social definition", iFILELINE, strKeyWord);
	CString strA = strLine.Mid (iPos + strKeyWord.GetLength ());
	strA.TrimLeft (" \t");
	if (strA.Left(1) != ":")
		Err ("Line %d: Expecting colon after %s in social definition", iFILELINE, strKeyWord);
	CString strB = strA.Mid (1);
	strB.TrimLeft (" \t");

	if (bQuoted)
		{
		if (strA.Left(1) != "\"")
			Err ("Line %d: Expecting opening double quotes after %s in social definition", iFILELINE, strKeyWord);
		strB.TrimLeft ("\"");
		if ((iPos = strB.Find ("\"")) == -1)
			Err ("Line %d: Expecting closing double quotes after %s in social definition", iFILELINE, strKeyWord);
		strB = strB.Left (iPos);
		}
	else
		{
		if ((iPos = strB.FindOneOf ("\t,]")) != -1)
			strB = strB.Left (iPos);
		}
	return strB;
	}


int GetVal (CString strLine, CString strKeyWord, int iMin=-999, int iMax=999)
	{
	int iVal = atoi ((LPCTSTR)GetStr (strLine, strKeyWord));
	if (iVal < iMin)
		Err ("Line %d: %s value in social definition is too small (>= %d)", iFILELINE, strKeyWord, iMin);
	if (iVal > iMax)
		Err ("Line %d: %s value in social definition is too big (<= %d)", iFILELINE, strKeyWord, iMax);
	return iVal;
	}


void SetField (char* szField, CString strSource, int iFieldSize)
	{
	strncpy (szField, (LPCTSTR)strSource, iFieldSize);
	for (int i=strSource.GetLength(); i<iFieldSize; i++)
		szField[i] = '\0';
	szField[iFieldSize-1] = '\0';
	}

// redo
static void ParseLoc (CString& strLine, int& iPage, int& iButton)
	{
	strLine.TrimLeft (" \t");
	int iPos = strLine.Find (".");
	if (iPos == -1)
		Err ("Line %d: Unknown Location in Social Header: %s", iFILELINE, strLine);
	CString strTmp = strLine.Left (iPos);
	iPage = atoi ((PSZ)(LPCTSTR)strTmp) - 1;
	strLine = strLine.Mid (iPos+1);
	iButton = atoi ((PSZ)(LPCTSTR)strLine) - 1;

	iPos = strLine.FindOneOf (" \t");
	if (iPos != -1)
		strLine = strLine.Mid (iPos+1);
	strLine.TrimLeft ();
	}

static void ParseSocialHeader (CString strLine, int& iPage, int& iButton, CString& strName, BOOL& bExtended)
	{
	strLine.TrimLeft (" \t[");

	if (bExtended = (strLine[0] == 'X' || strLine[0] == 'x'))
		strLine = strLine.Mid (1);
	if (strLine.Left (6).CompareNoCase ("Social"))
		Err ("Line %d: Unknown Section Header: %s", iFILELINE, strLine);
	strLine = strLine.Mid (6);
	ParseLoc (strLine, iPage, iButton);
	strLine.TrimRight (" \t]");
	strName = strLine;
	}


BOOL MakeOutFileName (PSZ pszOutFile, PSZ pszInFile)
	{
	strcpy (pszOutFile, pszInFile);
	PSZ p1;
	if ((p1 = strrchr (pszOutFile, '.')) && !stricmp (p1+1, "opt"))
		{
		*p1 = '\0';
		strcat (pszOutFile, ".txt");
		return TRUE;
		}
	if ((p1 = strrchr (pszOutFile, '.')) && !stricmp (p1+1, "txt"))
		{
		*p1 = '\0';
		strcat (pszOutFile, ".opt");
		return FALSE;
		}
	return Usage ();
	}

CString StrNum (int iIndex, PSZ* ppszStringArray, BOOL bSocial = FALSE)
	{
	CString strResult;
	for (int i=0; ppszStringArray && ppszStringArray[i]; i++)
		;
	if (iIndex < i && *ppszStringArray[iIndex])
		strResult = ppszStringArray[iIndex];
	else if (bSocial)
		{
		int iPage   = iIndex / SOCIAL_BUTTONS_PER_PAGE;
		int iButton = iIndex % SOCIAL_BUTTONS_PER_PAGE;

		CSoc* pSoc = cSocials[iPage] + iButton;
		if (*pSoc->pszName)
			strResult = pSoc->pszName;
		else
			strResult.Format ("%d.%d", iPage + 1, iButton + 1);
		}
	else
		strResult.Format ("%d", iIndex + 1);
	return strResult;
	}

int NumStr (CString strVal, PSZ* ppszStringList, BOOL bSocial = FALSE)
	{
	int iPage, iButton;

	strVal.TrimLeft ();
	strVal.TrimRight ();

	if (!bSocial) // not a spcial definition
		{
		if (strVal[0] >= '0' && strVal[0] <= '9')
			return atoi ((PSZ)(LPCTSTR)strVal) -1;
		for (int i=0; ppszStringList && ppszStringList[i]; i++)
			if (!strVal.CompareNoCase (ppszStringList[i]))
				return i;
		Err ("Line %d: cannot eval Hot Button definition: %s", iFILELINE, (LPCTSTR)strVal);
		}
	if (strVal[0] >= '0' && strVal[0] <= '9') // a Social definition specified by number
		{
		int iPos = strVal.Find ('.');
		if (iPos == -1)
			Err ("Line %d: cannot find dot in Hot Button definition", iFILELINE);
		CString strTmp = strVal.Left (iPos-1);
		iPage = NumStr (strTmp, NULL);
		strTmp = strVal.Mid (iPos+1);
		iButton = NumStr (strTmp, NULL);
		return iPage * SOCIAL_BUTTONS_PER_PAGE + iButton;
		}
	// a Social definition specified by name
	for (iPage = 0; iPage<SOCIAL_PAGES; iPage++)
		for (iButton = 0; iButton<SOCIAL_BUTTONS_PER_PAGE; iButton++)
			if (!strVal.CompareNoCase (cSocials[iPage][iButton].pszName))
				return iPage * SOCIAL_BUTTONS_PER_PAGE + iButton;
	Err ("Line %d: cannot eval Hot Button definition: %s", iFILELINE, (LPCTSTR)strVal);
	return 0;
	}

void HotButtonToName (CHB* pHB, CString& strName)
	{
	CString strType;
	CString strVal;

	strType = StrNum (pHB->iType, ppszTypes);
	switch (pHB->iType)
		{
		case 1: strVal = StrNum (pHB->iIndex, ppszMainCombat	);	break;  //"MainCombat",	
//		case 2: strVal = StrNum (pHB->iIndex, NULL				);	break;  //"ClassCombat",	
//		case 3: strVal = StrNum (pHB->iIndex, NULL				);	break;  //"Ability",	 	
		case 4: strVal = StrNum (pHB->iIndex, NULL, TRUE	   );	break;  //"Social",		
		case 5: strVal = StrNum (pHB->iIndex, ppszInventory	);	break;  //"Inventory",	
		case 6: strVal = StrNum (pHB->iIndex, ppszMain			);	break;  //"Main",			
		default:strVal = StrNum (pHB->iIndex, NULL				);	break;  //
		}
	strName.Format ("%s.%s", strType, strVal);
	return;
	}


//	given strName,
//	sets pHB->iType and pHB->iIndex
//
void HotButtonFromName (CHB* pHB, CString strData)
	{
	int iPos = strData.Find ('.');
	if (iPos == -1)
		Err ("Line %d: cannot find dot in Hot Button definition", iFILELINE);
	// Type ident on left of dot
	CString strType = strData.Left (iPos);
	strType.TrimLeft ();	strType.TrimRight ();
	int iType = NumStr (strType, ppszTypes);

	// Enum value on right of dot
	int iVal;
	CString strVal  = strData.Mid (iPos+1);
	strVal.TrimLeft (); strVal.TrimRight ();
	switch (iType)
		{
		case 1: iVal = NumStr (strVal, ppszMainCombat);	break;  //"MainCombat",	
//		case 2: iVal = NumStr (strVal, NULL				);	break;  //"ClassCombat",	
//		case 3: iVal = NumStr (strVal, NULL				);	break;  //"Ability",	 	
		case 4: iVal = NumStr (strVal, NULL, TRUE		);	break;  //"Social",		
		case 5: iVal = NumStr (strVal, ppszInventory	);	break;  //"Inventory",	
		case 6: iVal = NumStr (strVal, ppszMain		);	break;  //"Main",			
		default:iVal = NumStr (strVal, NULL				);	break;  //
		}
	pHB->iType  = iType;
	pHB->iIndex	= iVal;
	}


/////////////////////////////////////////////////////////////////////////////
//
//	Read/Write opt and text files
//
/////////////////////////////////////////////////////////////////////////////


static void ReadSocialsFromOpt (CFile* pFile)
	{
	pFile->Seek (SOCIAL_FILE_OFFSET, CFile::begin);
	for (int iPage=0; iPage<SOCIAL_PAGES; iPage++)
		for (int iButton=0; iButton<SOCIAL_BUTTONS_PER_PAGE; iButton++)
			pFile->Read (&cSocials[iPage][iButton], SOCIAL_STRUCT_SIZE);
	}


static void ReadSocialsFromTxt (CStdioFile* pFile)
	{
	int iSocialLine = SOCIAL_MAX_LINES;
	int iPage 		 = -1; 	 
	int iButton 	 = -1; 
	CString strLine, strName; 
	BOOL bExtended  = FALSE;

	pFile->Seek (0, CFile::begin);
	iFILELINE = 0;
	while (pFile->ReadString (strLine))
		{
		iFILELINE++;
		strLine.TrimLeft (" \t");
		if (strLine.Left (1) == ';')
			continue; // comment
		strLine.TrimRight(" \t");
		if ((!strLine.Left (7).CompareNoCase ("[Social")) || // start of new social button
 		    (!strLine.Left (8).CompareNoCase ("[XSocial")))  // start of new social button (extended format)
			{
			ParseSocialHeader (strLine, iPage, iButton, strName, bExtended);
			iSocialLine = 0;
			SetField (cSocials[iPage][iButton].pszName, strName, SOCIAL_NAME_SIZE);
			}
		else if (iSocialLine < SOCIAL_MAX_LINES)
			{
			if (bExtended)
				{
				SetField ((char*)(cSocials[iPage][iButton].pszLine[0]), strLine, SOCIAL_LINE_SIZE * SOCIAL_MAX_LINES);
				iSocialLine = SOCIAL_MAX_LINES;
				}
			else
				SetField (cSocials[iPage][iButton].pszLine[iSocialLine++], strLine, SOCIAL_LINE_SIZE);
			}
		}
	}

static void ReadHotButtonsFromOpt (CFile* pFile)
	{
	pFile->Seek (HOTBUTTON_FILE_OFFSET, CFile::begin);
	for (int iPage=0; iPage<HOTBUTTON_PAGES; iPage++)
		for (int iButton=0; iButton<HOTBUTTONS_PER_PAGE; iButton++)
			pFile->Read (&cHotButtons[iPage][iButton], HOTBUTTON_STRUCT_SIZE);
	}

static void ReadHotButtonsFromTxt (CStdioFile* pFile)
	{
	BOOL bReading = FALSE;
	pFile->Seek (0, CFile::begin);
	iFILELINE = 0;
	CString strLine;
	while (pFile->ReadString (strLine))
		{
		iFILELINE++;
		strLine.TrimLeft (" \t");
		if (strLine.Left (1) == ';' || strLine.IsEmpty ())
			continue; // comment
		if (strLine.Left (1) == '[' && bReading)
			return;
		if (!strLine.Left (11).CompareNoCase ("[HotButtons")) // start of new social button
			{
			bReading = TRUE;
			continue;
			}
		if (!bReading)
			continue;
		// Split on =
		int iPos = strLine.Find ('=');
		if (iPos == -1)
			Err ("Line %d: cannot find equals sign in Hot Button definition", iFILELINE);

		// left half is HB location
		CString strLoc = strLine.Left (iPos-1);
		int iPage;
		int iButton;
		ParseLoc (strLoc, iPage, iButton);

		// right half is value
		CString strVal = strLine.Mid (iPos+1);
		iPos = strVal.Find (';');
		if (iPos != -1)
			strVal = strVal.Left (iPos-1); // chop off inline comment
		strVal.TrimLeft ();
		strVal.TrimRight ();

		CHB* pHB = cHotButtons[iPage] + iButton;
		HotButtonFromName (pHB, strVal);
		}
	}

static void WriteSocialsToOpt (CFile* pFile)
	{
	pFile->Seek (SOCIAL_FILE_OFFSET, CFile::begin);
	for (int iPage=0; iPage<SOCIAL_PAGES; iPage++)
		for (int iButton=0; iButton<SOCIAL_BUTTONS_PER_PAGE; iButton++)
			pFile->Write (&cSocials[iPage][iButton], SOCIAL_STRUCT_SIZE);
	pFile->Seek (0, CFile::end);
	}

static void WriteSocialsToTxt (CStdioFile* pFile)
	{
	if (!bNO_COMMENTS)
		SocialMacroComments (pFile);

	for (int iPage=0; iPage<SOCIAL_PAGES; iPage++)
		for (int iButton=0; iButton<SOCIAL_BUTTONS_PER_PAGE; iButton++)
			{
			CSoc* pSoc = cSocials[iPage] + iButton;
			if (*pSoc->pszName)
				{
				BOOL bExtended = TRUE;
				for (int i=0; i<SOCIAL_LINE_SIZE; i++) // look to se if it is an extended social
					bExtended &= !!pSoc->pszLine[0][i];

				CString str;
				str.Format ("[%sSocial %d.%d %s]\n", (bExtended ? "X" : ""), iPage+1, iButton+1, pSoc->pszName);
				pFile->WriteString (str);
				for (int iLine=0; iLine<SOCIAL_MAX_LINES; iLine++)
					{
					pFile->WriteString (CString (pSoc->pszLine[iLine]) + "\n");
					if (bExtended)
						{
						pFile->WriteString ("\n");
						iLine = SOCIAL_MAX_LINES;
						}
					}
				}
			}
	}


static void WriteHotButtonsToOpt (CFile* pFile)
	{
	pFile->Seek (HOTBUTTON_FILE_OFFSET, CFile::begin);
	for (int iPage=0; iPage<HOTBUTTON_PAGES; iPage++)
		for (int iButton=0; iButton<HOTBUTTONS_PER_PAGE; iButton++)
			pFile->Write (&cHotButtons[iPage][iButton], HOTBUTTON_STRUCT_SIZE);
	pFile->Seek (0, CFile::end);
	}

static void WriteHotButtonsToTxt (CStdioFile* pFile)
	{
	if (!bNO_COMMENTS)
		HotButtonComments (pFile);

	pFile->WriteString ("[HotButtons]\n");
	for (int iPage=0; iPage<HOTBUTTON_PAGES; iPage++)
		for (int iButton=0; iButton<HOTBUTTONS_PER_PAGE; iButton++)
			{
			CHB* pHB = cHotButtons[iPage] + iButton;
			if (pHB->iIndex == 0xFFFF || pHB->iType == 0)
				continue;
			CString strLine;
			CString strHBName;
			HotButtonToName (pHB, strHBName);
			strLine.Format ("%d.%d = %s\n", iPage+1, iButton+1, strHBName);
			pFile->WriteString (strLine);
			}
	pFile->WriteString ("\n\n");
	}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void DecodeOpt (PSZ pszInFile, PSZ pszOutFile)
	{
	CFile		  cInFile;
	CStdioFile cOutFile;

	if (!cInFile.Open (pszInFile, CFile::modeRead))
		Err ("Could not open input file %s", pszInFile);
	if (!cOutFile.Open (pszOutFile, CFile::modeWrite | CFile::modeCreate))
		Err ("Could not open output file %s", pszOutFile);

	if (!bNO_COMMENTS)
		GeneralComments (&cOutFile);

	ReadHotButtonsFromOpt (&cInFile);
	ReadSocialsFromOpt (&cInFile);
	WriteSocialsToTxt (&cOutFile);
	WriteHotButtonsToTxt (&cOutFile);

	cInFile.Close ();
	cOutFile.Close ();
	}

void EncodeOpt (PSZ pszInFile, PSZ pszOutFile)
	{
	CStdioFile cInFile;
	CFile 	  cOutFile;

	if (!cInFile.Open (pszInFile, CFile::modeRead))
		Err ("Could not open input file %s", pszInFile);
	if (!cOutFile.Open (pszOutFile, CFile::modeReadWrite))
		Err ("Could not open output file %s", pszOutFile);

	ReadSocialsFromTxt (&cInFile);
	ReadHotButtonsFromTxt (&cInFile);

	if (!bHOTBUTTONS_ONLY)
		WriteSocialsToOpt (&cOutFile);
	if (!bSOCIALS_ONLY)
		WriteHotButtonsToOpt (&cOutFile);

	cInFile.Close ();
	cOutFile.Close ();
	}

void Test1 (PSZ pszInFile, PSZ pszOutFile)
	{
	CStdioFile cInFile;
	CStdioFile cOutFile;

	if (!cInFile.Open (pszInFile, CFile::modeRead))
		Err ("Could not open input file %s", pszInFile);
	if (!cOutFile.Open (pszOutFile, CFile::modeWrite | CFile::modeCreate))
		Err ("Could not open output file %s", pszOutFile);

	if (!bNO_COMMENTS)
		GeneralComments (&cOutFile);

	ReadSocialsFromTxt 	(&cInFile);
	ReadHotButtonsFromTxt(&cInFile);
	WriteSocialsToTxt  	(&cOutFile);
	WriteHotButtonsToTxt (&cOutFile);

	cInFile.Close ();
	cOutFile.Close ();
	}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
	{
	CFile cFile;

	AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0);

   ArgBuildBlk ("? *^Help *^Debug ^*Update ^*SocialsOnly ^*HotButtonsOnly ^*NoComments");

   if (ArgFillBlk (argv))
      Err (ArgGetErr (), 0, 0);

   if (ArgIs ("help") || ArgIs ("?") || !ArgIs (NULL))
      Usage ();

	bSOCIALS_ONLY	  = ArgIs ("SocialsOnly");
	bHOTBUTTONS_ONLY = ArgIs ("HotButtonsOnly");
	bNO_COMMENTS	  = ArgIs ("NoComments");

	pszInFile = ArgGet (NULL, 0);
	char szOutFile [256];
	BOOL bExtract = MakeOutFileName (szOutFile, pszInFile);
	if (ArgGet (NULL, 1))
		strcpy (szOutFile, ArgGet (NULL, 1));

	if (ArgIs ("Debug") && !bExtract && ArgGet (NULL, 1))
		Test1 (pszInFile, szOutFile);
	else if (ArgIs ("Debug"))
		Err ("Use Debug option with 2 .txt files only");
	else if (bExtract)
		{
		DecodeOpt (pszInFile, szOutFile);
		printf ("Extracted macro's from %s to %s\n", pszInFile, szOutFile);
		}
	else
		{
		EncodeOpt (pszInFile, szOutFile);
		printf ("Set macro's from %s to %s\n", pszInFile, szOutFile);
		}
	return 0;
	}

