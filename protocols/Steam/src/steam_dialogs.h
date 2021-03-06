#ifndef _STEAM_DIALOGS_H_
#define _STEAM_DIALOGS_H_

typedef CProtoDlgBase<CSteamProto> CSteamDlgBase;

#define DIALOG_RESULT_OK 1

/////////////////////////////////////////////////////////////////////////////////

class CSteamPasswordEditor : public CSteamDlgBase
{
private:
	CCtrlEdit m_password;
	CCtrlCheck m_savePermanently;

	CCtrlButton m_ok;

protected:
	void OnInitDialog();
	void OnOk(CCtrlButton*);
	void OnClose();

public:
	CSteamPasswordEditor(CSteamProto *proto);
};

/////////////////////////////////////////////////////////////////////////////////

class CSteamGuardDialog : public CSteamDlgBase
{
private:
	char m_domain[32];
	char m_guardCode[6];

	CCtrlEdit m_text;
	CCtrlButton m_ok;
	CCtrlHyperlink m_link;

protected:
	void OnInitDialog();
	void OnOk(CCtrlButton*);
	void OnClose();

public:
	CSteamGuardDialog(CSteamProto *proto, const char *domain);

	const char *GetGuardCode();
};

/////////////////////////////////////////////////////////////////////////////////

class CSteamTwoFactorDialog : public CSteamDlgBase
{
private:
	char m_twoFactorCode[6];

	CCtrlEdit m_text;
	CCtrlButton m_ok;

protected:
	void OnInitDialog();
	void OnOk(CCtrlButton*);
	void OnClose();

public:
	CSteamTwoFactorDialog(CSteamProto *proto);

	const char *GetTwoFactorCode();
};

/////////////////////////////////////////////////////////////////////////////////

class CSteamCaptchaDialog : public CSteamDlgBase
{
private:
	char m_captchaText[7];

	uint8_t *m_captchaImage;
	int m_captchaImageSize;

	CCtrlEdit m_text;
	CCtrlButton m_ok;

protected:
	void OnInitDialog();
	void OnOk(CCtrlButton*);
	void OnClose();

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	CSteamCaptchaDialog(CSteamProto *proto, const uint8_t *captchaImage, int captchaImageSize);
	~CSteamCaptchaDialog();

	const char *GetCaptchaText();
};

#endif //_STEAM_DIALOGS_H_