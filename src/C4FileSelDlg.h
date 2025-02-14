/*
 * LegacyClonk
 *
 * Copyright (c) RedWolf Design
 * Copyright (c) 2008, Sven2
 * Copyright (c) 2017-2020, The LegacyClonk Team and contributors
 *
 * Distributed under the terms of the ISC license; see accompanying file
 * "COPYING" for details.
 *
 * "Clonk" is a registered trademark of Matthes Bender, used with permission.
 * See accompanying file "TRADEMARK" for details.
 *
 * To redistribute this file separately, substitute the full license texts
 * for the above references.
 */

// file selection dialogs

#pragma once

#include "C4Components.h"
#include "C4Gui.h"

// callback handler for file selection dialog
class C4FileSel_BaseCB
{
public:
	C4FileSel_BaseCB() {}
	virtual ~C4FileSel_BaseCB() {}

public:
	virtual void OnFileSelected(const char *szFilename) = 0;
};

template <class CB> class C4FileSel_CBEx : public C4FileSel_BaseCB
{
public:
	typedef void (CB::*FileSelFunc)(const char *szFilename, int32_t idToken);

private:
	CB *pCBClass;
	FileSelFunc SelFunc;
	int32_t idToken;

public:
	virtual void OnFileSelected(const char *szFilename) override
	{
		if (pCBClass && SelFunc)(pCBClass->*SelFunc)(szFilename, idToken);
	}

public:
	C4FileSel_CBEx(CB *pCBClass, FileSelFunc SelFunc, int32_t idToken) : pCBClass(pCBClass), SelFunc(SelFunc), idToken(idToken) {}
};

// dialog to select one or more files
class C4FileSelDlg : public C4GUI::Dialog
{
public:
	class ListItem : public C4GUI::Control
	{
	protected:
		StdStrBuf sFilename; // full path to file

		virtual bool IsFocusOnClick() override { return false; } // do not focus; keep focus on listbox

	public:
		ListItem(const char *szFilename);
		virtual ~ListItem();

		const char *GetFilename() { return sFilename.getData(); }

		// multisel-checkbox-options
		virtual bool IsChecked() const { return false; }
		virtual void SetChecked(bool fChecked) {}
		virtual bool IsGrayed() const { return false; }
		virtual bool UserToggleCheck() { return false; }
	};

	class DefaultListItem : public ListItem
	{
	private:
		typedef ListItem BaseClass;
		class C4GUI::Icon *pIco; class C4GUI::Label *pLbl;
		class C4GUI::CheckBox *pCheck;
		class C4KeyBinding *pKeyCheck; // space activates/deactivates selected file
		bool fGrayed;

	protected:
		void UpdateOwnPos() override;

	public:
		DefaultListItem(const char *szFilename, bool fTruncateExtension, bool fCheckbox, bool fGrayed, C4GUI::Icons eIcon);
		virtual ~DefaultListItem();

		virtual bool IsChecked() const override;
		virtual void SetChecked(bool fChecked) override;
		virtual bool IsGrayed() const override { return fGrayed; }
		virtual bool UserToggleCheck() override;
	};

private:
	typedef C4GUI::Dialog BaseClass;

	C4KeyBinding *pKeyRefresh, *pKeyEnterOverride;

	C4GUI::ComboBox *pLocationComboBox;
	C4GUI::ListBox *pFileListBox;
	C4GUI::TextWindow *pSelectionInfoBox;
	C4GUI::Button *btnOK;

	StdStrBuf sTitle; // dlg title

	StdStrBuf sPath; // current path
	struct Location
	{
		StdStrBuf sName;
		StdStrBuf sPath;
	};
	Location *pLocations;
	int32_t iLocationCount;
	int32_t iCurrentLocationIndex;

	ListItem *pSelection;
	C4FileSel_BaseCB *pSelCallback;

	void UpdateFileList(); // rebuild file listbox from sPath

protected:
	virtual void OnShown() override;
	virtual void UserClose(bool fOK) override; // allow OK only if something is sth is selected
	virtual void OnClosed(bool fOK) override; // callback when dlg got closed

	virtual const char *GetFileMask() const { return nullptr; }
	virtual bool IsMultiSelection() const { return false; } // if true, files are checked/unchecked using checkboxes
	virtual bool IsItemGrayed(const char *szFilename) const { return false; }
	virtual void UpdateSelection();
	virtual bool HasNoneItem() const { return false; } // if true, an empty item can be selected
	virtual bool HasPreviewArea() const { return true; }
	virtual bool HasExtraOptions() const { return false; }
	virtual void AddExtraOptions(const C4Rect &rcOptionsRect) {}
	virtual C4GUI::Icons GetFileItemIcon() const { return C4GUI::Ico_None; }
	virtual int32_t GetFileSelColWidth() const { return 0; } // width of each file selection element; 0 for default all listbox

	void OnSelChange(class C4GUI::Element *pEl) { UpdateSelection(); }
	void OnSelDblClick(class C4GUI::Element *pEl);
	bool KeyRefresh() { UpdateFileList(); return true; }
	void OnLocationComboFill(C4GUI::ComboBox_FillCB *pFiller);
	bool OnLocationComboSelChange(C4GUI::ComboBox *pForCombo, int32_t idNewSelection);

	void AddLocation(const char *szName, const char *szPath); // add location to be shown in dropdown list
	void AddCheckedLocation(const char *szName, const char *szPath); // add location to be shown in dropdown list, only if path exists and isn't added yet
	int32_t GetCurrentLocationIndex() const;
	void SetCurrentLocation(int32_t idx, bool fRefresh);

	virtual ListItem *CreateListItem(const char *szFilename);
	virtual void BeginFileListUpdate() {}
	virtual void EndFileListUpdate() {}

public:
	C4FileSelDlg(const char *szRootPath, const char *szTitle, C4FileSel_BaseCB *pSelCallback, bool fInitElements = true);
	virtual ~C4FileSelDlg();
	void InitElements();

	void SetPath(const char *szNewPath, bool fRefresh = true);
	void SetSelection(const std::vector<std::string> &newSelection, bool filenameOnly);
	std::vector<std::string> GetSelection(const std::vector<std::string> &fixedSelection, bool filenameOnly) const; // get single selected file for single selection dlg ';'-separated list for multi selection dlg
};

// dialog to select a player file
class C4PlayerSelDlg : public C4FileSelDlg
{
protected:
	virtual const char *GetFileMask() const override { return C4CFN_PlayerFiles; }
	virtual C4GUI::Icons GetFileItemIcon() const override { return C4GUI::Ico_Player; }

public:
	C4PlayerSelDlg(C4FileSel_BaseCB *pSelCallback);
};

// dialog to select definition files
class C4DefinitionSelDlg : public C4FileSelDlg
{
private:
	std::vector<std::string> fixedSelection; // initial selection which cannot be deselected

protected:
	virtual void OnShown() override;
	virtual const char *GetFileMask() const override { return C4CFN_DefFiles; }
	virtual bool IsMultiSelection() const override { return true; }
	virtual bool IsItemGrayed(const char *szFilename) const override;
	virtual C4GUI::Icons GetFileItemIcon() const override { return C4GUI::Ico_Definition; }

public:
	C4DefinitionSelDlg(C4FileSel_BaseCB *pSelCallback, const std::vector<std::string> &fixedSelection);

	static bool SelectDefinitions(C4GUI::Screen *pOnScreen, std::vector<std::string> &selection);
};

// dialog to select portrait files
class C4PortraitSelDlg : public C4FileSelDlg
{
public:
	enum { ImagePreviewSize = 100 };

private:
	class ListItem : public C4FileSelDlg::ListItem
	{
	private:
		bool fError; // loading error
		bool fLoaded; // image loaded but not yet scaled
		C4FacetExSurface fctImage; // portrait, if loaded
		C4FacetExSurface fctLoadedImage; // image as loaded by background thread. Must be scaled by main thread
		StdStrBuf sFilenameLabelText;

	protected:
		void DrawElement(C4FacetEx &cgo) override;

	public:
		ListItem(const char *szFilename);

		void Load();
	};

	// portrait loader thread
	class ImageLoader
	{
	private:
		std::list<ListItem *> LoadItems; // items to be loaded by this thread

	public:
		void ClearLoadItems(); // stops thread
		void AddLoadItem(ListItem *pItem); // not to be called when thread is running!

	public:
		virtual void Execute();
	};

private:
	C4GUI::CheckBox *pCheckSetPicture, *pCheckSetBigIcon;
	bool fDefSetPicture, fDefSetBigIcon;

	ImageLoader ImageLoader;

protected:
	void OnClosed(bool fOK) override;
	virtual const char *GetFileMask() const override { return C4CFN_ImageFiles; }
	virtual bool HasNoneItem() const override { return true; } // if true, a special <none> item can be selected
	virtual bool HasPreviewArea() const override { return false; } // no preview area. Preview images directly
	virtual bool HasExtraOptions() const override { return true; }
	virtual void AddExtraOptions(const C4Rect &rcOptionsRect) override;
	virtual int32_t GetFileSelColWidth() const override { return ImagePreviewSize; } // width of each file selection element

	virtual C4FileSelDlg::ListItem *CreateListItem(const char *szFilename) override;
	virtual void BeginFileListUpdate() override;

	virtual void OnIdle() override;

public:
	C4PortraitSelDlg(C4FileSel_BaseCB *pSelCallback, bool fSetPicture, bool fSetBigIcon);

	bool IsSetPicture() const { return pCheckSetPicture ? pCheckSetPicture->GetChecked() : fDefSetPicture; }
	bool IsSetBigIcon() const { return pCheckSetBigIcon ? pCheckSetBigIcon->GetChecked() : fDefSetBigIcon; }

	static bool SelectPortrait(C4GUI::Screen *pOnScreen, std::string &selection, bool *pfSetPicture, bool *pfSetBigIcon);
};
