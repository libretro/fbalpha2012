// Burner Cheat search dialog module, added by regret
// TODO: select cpu and limit address

/* changelog:
 update 1: create (ref: snes9x-rr)
*/

#ifndef NO_CHEATSEARCH

#include "burner.h"
#include "cheat.h"

// definitions
typedef enum
{
	CS_LESS_THAN, CS_GREATER_THAN, CS_LESS_THAN_OR_EQUAL,
	CS_GREATER_THAN_OR_EQUAL, CS_EQUAL, CS_NOT_EQUAL
} CheatComparisonType;

typedef enum
{
	CS_8_BITS, CS_16_BITS, CS_24_BITS, CS_32_BITS
} CheatDataSize;

#define BIT_CLEAR(a, v) \
(a)[(v) >> 5] &= ~(1 << ((v) & 31))

#define BIT_SET(a, v) \
(a)[(v) >> 5] |= 1 << ((v) & 31)

#define TEST_BIT(a, v) \
((a)[(v) >> 5] & (1 << ((v) & 31)))

#define _C(c, a, b) \
((c) == CS_LESS_THAN ? (a) < (b) : \
 (c) == CS_GREATER_THAN ? (a) > (b) : \
 (c) == CS_LESS_THAN_OR_EQUAL ? (a) <= (b) : \
 (c) == CS_GREATER_THAN_OR_EQUAL ? (a) >= (b) : \
 (c) == CS_EQUAL ? (a) == (b) : \
 (a) != (b))

#define _D(s, m, o) \
((s) == CS_8_BITS ? (UINT8) (*((m) + (o))) : \
 (s) == CS_16_BITS ? ((UINT16) (*((m) + (o)) + (*((m) + (o) + 1) << 8))) : \
 (s) == CS_24_BITS ? ((UINT32) (*((m) + (o)) + (*((m) + (o) + 1) << 8) + (*((m) + (o) + 2) << 16))) : \
((UINT32)  (*((m) + (o)) + (*((m) + (o) + 1) << 8) + (*((m) + (o) + 2) << 16) + (*((m) + (o) + 3) << 24))))

#define _DS(s, m, o) \
((s) == CS_8_BITS ? ((INT8) *((m) + (o))) : \
 (s) == CS_16_BITS ? ((INT16) (*((m) + (o)) + (*((m) + (o) + 1) << 8))) : \
 (s) == CS_24_BITS ? (((INT32) ((*((m) + (o)) + (*((m) + (o) + 1) << 8) + (*((m) + (o) + 2) << 16)) << 8)) >> 8): \
 ((INT32) (*((m) + (o)) + (*((m) + (o) + 1) << 8) + (*((m) + (o) + 2) << 16) + (*((m) + (o) + 3) << 24))))

// variables
HANDLE cheatSearchThread;
DWORD cheatSearchThreadID;

HWND cheatSearchDlg = NULL;
static HWND cheatSearchList = NULL;

static inline void scanAddress(const TCHAR* str, unsigned int* value)
{
	int plus = (*str == '0' && _totlower(str[1]) == 'x') ? 2 : 0;
	_stscanf(str + plus, _T("%x"), value);
}

static inline unsigned int cheatCount(int byteSub)
{
	unsigned int count = 0;
	for (unsigned int i = 0; i < cheatSearchInfo.size - byteSub; i++) {
		if (TEST_BIT(cheatSearchInfo.ALL_BITS, i)) {
			count++;
		}
	}
	return count;
}

static void setListCount(int bytes)
{
	if (cheatSearchList) {
		unsigned int c = cheatCount(bytes);
		ListView_SetItemCount(cheatSearchList, c);
	}
}

static inline int getAdddressIndex(CheatDataSize bytes, int item)
{
	unsigned int i;
	for (i = 0; i < (cheatSearchInfo.size - bytes) && item >= 0; i++) {
		if (TEST_BIT(cheatSearchInfo.ALL_BITS, i)) {
			item--;
		}
	}

	if (i >= cheatSearchInfo.size && item >= 0) {
		return -1;
	}

	return i - 1;
}

static inline void searchForChange(CheatComparisonType cmp, CheatDataSize size, UINT8 is_signed, UINT8 update)
{
	int l;

	switch (size) {
		case CS_8_BITS: l = 0; break;
		case CS_16_BITS: l = 1; break;
		case CS_24_BITS: l = 2; break;
		default:
		case CS_32_BITS: l = 3; break;
	}

	cheatSearchCopyRAM(cheatSearchInfo.RAM);

	if (is_signed) {
		for (unsigned int i = 0; i < cheatSearchInfo.size - l; i++) {
			if (TEST_BIT(cheatSearchInfo.ALL_BITS, i)) {
				if (_C(cmp, _DS(size, cheatSearchInfo.RAM, i), _DS(size, cheatSearchInfo.CRAM, i))) {
					if (update) {
						cheatSearchInfo.CRAM[i] = cheatSearchGet(i);
					}
				} else {
					BIT_CLEAR(cheatSearchInfo.ALL_BITS, i);
				}
			}
		}
	} else {
		for (unsigned int i = 0; i < cheatSearchInfo.size - l; i++) {
			if (TEST_BIT(cheatSearchInfo.ALL_BITS, i)) {
				if (_C(cmp, _D(size, cheatSearchInfo.RAM, i), _D(size, cheatSearchInfo.CRAM, i))) {
					if (update) {
						cheatSearchInfo.CRAM[i] = cheatSearchGet(i);
					}
				} else {
					BIT_CLEAR(cheatSearchInfo.ALL_BITS, i);
				}
			}
		}
	}

	for (unsigned int i = cheatSearchInfo.size - l; i < cheatSearchInfo.size; i++) {
		BIT_CLEAR(cheatSearchInfo.ALL_BITS, i);
	}
}

static inline void searchForValue(CheatComparisonType cmp, CheatDataSize size,
							UINT32 value, UINT8 is_signed, UINT8 update)
{
	int l;

	switch (size) {
		case CS_8_BITS: l = 0; break;
		case CS_16_BITS: l = 1; break;
		case CS_24_BITS: l = 2; break;
		default:
		case CS_32_BITS: l = 3; break;
	}

	cheatSearchCopyRAM(cheatSearchInfo.RAM);

	if (is_signed) {
		for (unsigned int i = 0; i < cheatSearchInfo.size - l; i++) {
			if (TEST_BIT(cheatSearchInfo.ALL_BITS, i)) {
				if (_C(cmp, _DS(size, cheatSearchInfo.RAM, i), (INT32)value)) {
					if (update) {
						cheatSearchInfo.CRAM[i] = cheatSearchInfo.RAM[i];
					}
				} else {
					BIT_CLEAR(cheatSearchInfo.ALL_BITS, i);
				}
			}
		}
	} else {
		for (unsigned int i = 0; i < cheatSearchInfo.size - l; i++) {
			if (TEST_BIT(cheatSearchInfo.ALL_BITS, i)) {
				if (_C(cmp, _D(size, cheatSearchInfo.RAM, i), value)) {
					if (update) {
						cheatSearchInfo.CRAM[i] = cheatSearchInfo.RAM[i];
					}
				} else {
					BIT_CLEAR(cheatSearchInfo.ALL_BITS, i);
				}
			}
		}
	}

	for (unsigned int i = cheatSearchInfo.size - l; i < cheatSearchInfo.size; i++) {
		BIT_CLEAR(cheatSearchInfo.ALL_BITS, i);
	}
}

static inline void searchForAddress(CheatComparisonType cmp, CheatDataSize size, UINT32 value, UINT8 update)
{
	int l;

	switch (size) {
		case CS_8_BITS: l = 0; break;
		case CS_16_BITS: l = 1; break;
		case CS_24_BITS: l = 2; break;
		default:
		case CS_32_BITS: l = 3; break;
	}

	for (unsigned int i = 0; i < cheatSearchInfo.size - l; i++)
	{
		if (TEST_BIT(cheatSearchInfo.ALL_BITS, i) && _C(cmp, i, (INT32)value)) {
			if (update) {
				cheatSearchInfo.CRAM[i] = cheatSearchGet(i);
			}
		} else {
			BIT_CLEAR(cheatSearchInfo.ALL_BITS, i);
		}
	}

	for (unsigned int i = cheatSearchInfo.size - l; i < cheatSearchInfo.size; i++) {
		BIT_CLEAR(cheatSearchInfo.ALL_BITS, i);
	}
}

static inline BOOL testRange(int val_type, CheatDataSize bytes, UINT32 value)
{
	if (val_type != 2) {
		if (bytes == CS_8_BITS) {
			if (value < 256)
				return TRUE;
			else
				return FALSE;
		}
		else if (bytes == CS_16_BITS) {
			if (value < 65536)
				return TRUE;
			else
				return FALSE;
		}
		else if (bytes == CS_24_BITS)
		{
			if (value < 16777216)
				return TRUE;
			else
				return FALSE;
		}
		// if it reads in, it's a valid 32-bit unsigned!
		return TRUE;
	} else {
		if (bytes == CS_8_BITS) {
			if ((INT32)value < 128 && (INT32)value >= -128)
				return TRUE;
			else
				return FALSE;
		}
		else if (bytes == CS_16_BITS)
		{
			if ((INT32)value<32768 && (INT32)value >= -32768)
				return TRUE;
			else
				return FALSE;
		}
		else if (bytes == CS_24_BITS)
		{
			if ((INT32)value < 8388608 && (INT32)value >= -8388608)
				return TRUE;
			else
				return FALSE;
		}
		// should be handled by sscanf
		return TRUE;
	}
}

static void resetCheatSearch()
{
	cheatSearchCopyRAM(cheatSearchInfo.CRAM);
	memset(cheatSearchInfo.ALL_BITS, 0xffffffff, sizeof(int) * (cheatSearchInfo.size >> 5));
}

void updateCheatSearch()
{
	if (cheatSearchList) {
		int top = ListView_GetTopIndex(cheatSearchList);
		int count = ListView_GetCountPerPage(cheatSearchList) + 1;
		ListView_RedrawItems(cheatSearchList, top, top + count);
	}
}

// search dialog
static void initListBox()
{
	if (!cheatSearchList) {
		return;
	}

	ListView_SetExtendedListViewStyle(cheatSearchList, LVS_EX_FULLROWSELECT);

	int i = 0;
	LVCOLUMN col;
	memset(&col, 0, sizeof(LVCOLUMN));
	col.mask = LVCF_FMT | LVCF_ORDER | LVCF_TEXT| LVCF_WIDTH | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;

	col.iOrder = i;
	col.iSubItem = i;
	col.cx = 70;
	col.pszText = FBALoadStringEx(IDS_CHEATSEARCH_ADDRESS);
	ListView_InsertColumn(cheatSearchList, i, &col);
	i++;

	col.iOrder = i;
	col.iSubItem = i;
	col.cx = 104;
	col.pszText = FBALoadStringEx(IDS_CHEATSEARCH_VALUE);
	ListView_InsertColumn(cheatSearchList, i, &col);
	i++;

	col.iOrder = i;
	col.iSubItem = i;
	col.cx = 104;
	col.pszText = FBALoadStringEx(IDS_CHEATSEARCH_PREV);
	ListView_InsertColumn(cheatSearchList, i, &col);
}

static LRESULT customDraw(LPARAM lParam)
{
	LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;

	switch (lplvcd->nmcd.dwDrawStage) {
		case CDDS_PREPAINT :
			return CDRF_NOTIFYITEMDRAW;

		case CDDS_ITEMPREPAINT: {
			int rv = CDRF_DODEFAULT;
			if (lplvcd->nmcd.dwItemSpec % 2) {
				// alternate the background color slightly
				lplvcd->clrTextBk = RGB(248,248,255);
				rv = CDRF_NEWFONT;
			}
			return rv;
		}
		break;
	}
	return CDRF_DODEFAULT;
}

static INT_PTR CALLBACK cheatSearchDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static CheatDataSize bytes;
	static int val_type;
	static int use_entered;
	static CheatComparisonType comp_type;

	if (msg == WM_INITDIALOG) {
		cheatSearchDlg = hwndDlg;
		cheatSearchList = GetDlgItem(hwndDlg, IDC_ADDYS);

		if (!kNetGame && bAutoPause) {
			bRunPause = 1;
		}

		cheatSearchInit();

		if (val_type == 0) {
			val_type = 1;
		}

		// defaults
		SendDlgItemMessage(hwndDlg, IDC_LESS_THAN, BM_SETCHECK, BST_CHECKED, 0);
		if (use_entered == 0) {
			SendDlgItemMessage(hwndDlg, IDC_PREV, BM_SETCHECK, BST_CHECKED, 0);
		}
		else if (use_entered == 1) {
			SendDlgItemMessage(hwndDlg, IDC_ENTERED, BM_SETCHECK, BST_CHECKED, 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_VALUE_ENTER), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ENTER_LABEL), TRUE);
		}
		else if (use_entered == 2) {
			SendDlgItemMessage(hwndDlg, IDC_ENTEREDADDRESS, BM_SETCHECK, BST_CHECKED, 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_VALUE_ENTER), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ENTER_LABEL), TRUE);
		}
		SendDlgItemMessage(hwndDlg, IDC_UNSIGNED, BM_SETCHECK, BST_CHECKED, 0);
		SendDlgItemMessage(hwndDlg, IDC_1_BYTE, BM_SETCHECK, BST_CHECKED, 0);

		if (comp_type == CS_GREATER_THAN) {
			SendDlgItemMessage(hwndDlg, IDC_LESS_THAN, BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwndDlg, IDC_GREATER_THAN, BM_SETCHECK, BST_CHECKED, 0);
		}
		else if (comp_type == CS_LESS_THAN_OR_EQUAL) {
			SendDlgItemMessage(hwndDlg, IDC_LESS_THAN, BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwndDlg, IDC_LESS_THAN_EQUAL, BM_SETCHECK, BST_CHECKED, 0);
		}
		else if (comp_type == CS_GREATER_THAN_OR_EQUAL) {
			SendDlgItemMessage(hwndDlg, IDC_LESS_THAN, BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwndDlg, IDC_GREATER_THAN_EQUAL, BM_SETCHECK, BST_CHECKED, 0);
		}
		else if (comp_type == CS_EQUAL) {
			SendDlgItemMessage(hwndDlg, IDC_LESS_THAN, BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwndDlg, IDC_EQUAL, BM_SETCHECK, BST_CHECKED, 0);
		}
		else if (comp_type == CS_NOT_EQUAL) {
			SendDlgItemMessage(hwndDlg, IDC_LESS_THAN, BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwndDlg, IDC_NOT_EQUAL, BM_SETCHECK, BST_CHECKED, 0);
		}

		if (val_type == 2) {
			SendDlgItemMessage(hwndDlg, IDC_UNSIGNED, BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwndDlg, IDC_SIGNED, BM_SETCHECK, BST_CHECKED, 0);
		}
		else if (val_type == 3) {
			SendDlgItemMessage(hwndDlg, IDC_UNSIGNED, BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwndDlg, IDC_HEX, BM_SETCHECK, BST_CHECKED, 0);
		}

		if (bytes == CS_16_BITS) {
			SendDlgItemMessage(hwndDlg, IDC_1_BYTE, BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwndDlg, IDC_2_BYTE, BM_SETCHECK, BST_CHECKED, 0);
		}
		else if (bytes == CS_24_BITS) {
			SendDlgItemMessage(hwndDlg, IDC_1_BYTE, BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwndDlg, IDC_3_BYTE, BM_SETCHECK, BST_CHECKED, 0);
		}
		else if (bytes == CS_32_BITS) {
			SendDlgItemMessage(hwndDlg, IDC_1_BYTE, BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(hwndDlg, IDC_4_BYTE, BM_SETCHECK, BST_CHECKED, 0);
		}

		initListBox();

		setListCount(bytes);

		return TRUE;
	}

	if (msg == WM_DESTROY) {
		cheatSearchExit();
		cheatSearchDlg = NULL;
		cheatSearchList = NULL;
		return FALSE;
	}

	if (msg == WM_NOTIFY) {
		if (wParam == IDC_ADDYS) {
			LPNMHDR nmh = (LPNMHDR)lParam;

			if (nmh->code == NM_CUSTOMDRAW) {
				SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, customDraw(lParam));
				return TRUE;
			}
			else if (nmh->code == LVN_GETDISPINFO) {
				static TCHAR buf[12]; // the following code assumes this variable is static
				NMLVDISPINFO* nmlvdi = (NMLVDISPINFO*)lParam;

				int i = getAdddressIndex(bytes, nmlvdi->item.iItem);
				if (i < 0) {
					return FALSE;
				}

				if (nmlvdi->item.iSubItem == 0) {
					_stprintf(buf, _T("%08X"), i);
					nmlvdi->item.pszText = buf;
				}
				else if (nmlvdi->item.iSubItem == 1 || nmlvdi->item.iSubItem == 2) {
					int value = 0;

					if (nmlvdi->item.iSubItem == 1) {
						// current value
						for (int r = 0; r <= bytes; r++) {
							value += (cheatSearchGet(i + r) << (8 * r));
						}
					}
					else if (nmlvdi->item.iSubItem == 2) {
						// previous value
						for (int r = 0; r <= bytes; r++) {
							value += (cheatSearchInfo.CRAM[i + r] << (8 * r));
						}
					}

					// needs to account for size
					switch (val_type) {
						case 1:
							_stprintf(buf, _T("%u"), value);
							break;

						case 3:
							switch(bytes) {
								default:
								case CS_8_BITS: _stprintf(buf, _T("%02X"), value & 0xFF); break;
								case CS_16_BITS: _stprintf(buf, _T("%04X"), value & 0xFFFF); break;
								case CS_24_BITS: _stprintf(buf, _T("%06X"), value & 0xFFFFFF); break;
								case CS_32_BITS: _stprintf(buf, _T("%08X"), value); break;
							}
							break;

						case 2:
							switch (bytes) {
								default:
								case CS_8_BITS:
									if (value - 128 < 0) {
										_stprintf(buf, _T("%d"), value & 0xFF);
									} else {
										_stprintf(buf, _T("%d"), value - 256);
									}
									break;
								case CS_16_BITS:
									if (value - 32768 < 0) {
										_stprintf(buf, _T("%d"), value & 0xFFFF);
									} else {
										_stprintf(buf, _T("%d"), value - 65536);
									}
									break;
								case CS_24_BITS:
									if (value - 0x800000 < 0) {
										_stprintf(buf, _T("%d"), value & 0xFFFFFF);
									} else {
										_stprintf(buf, _T("%d"), value - 0x1000000);
									}
									break;
								case CS_32_BITS:
									_stprintf(buf, _T("%d"), value);
									break;
							}
							break;
					}

					nmlvdi->item.pszText = buf;
				}
			}
			else if (nmh->code == (UINT)LVN_ITEMACTIVATE || nmh->code == (UINT)NM_CLICK) {
				BOOL enable = TRUE;
				if (-1 == ListView_GetSelectionMark(nmh->hwndFrom)) {
					enable = FALSE;
				}
				EnableWindow(GetDlgItem(hwndDlg, IDC_C_ADD), enable);
			}
		}
		return FALSE;
	}

	if (msg == WM_COMMAND) {
		switch (LOWORD(wParam)) {
			case IDC_LESS_THAN:
			case IDC_GREATER_THAN:
			case IDC_LESS_THAN_EQUAL:
			case IDC_GREATER_THAN_EQUAL:
			case IDC_EQUAL:
			case IDC_NOT_EQUAL:
				if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_LESS_THAN))
					comp_type = CS_LESS_THAN;
				else if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_GREATER_THAN))
					comp_type = CS_GREATER_THAN;
				else if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_LESS_THAN_EQUAL))
					comp_type = CS_LESS_THAN_OR_EQUAL;
				else if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_GREATER_THAN_EQUAL))
					comp_type = CS_GREATER_THAN_OR_EQUAL;
				else if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_EQUAL))
					comp_type = CS_EQUAL;
				else if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_NOT_EQUAL))
					comp_type = CS_NOT_EQUAL;
				break;

			case IDC_1_BYTE:
			case IDC_2_BYTE:
			case IDC_3_BYTE:
			case IDC_4_BYTE:
				if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_1_BYTE))
					bytes = CS_8_BITS;
				else if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_2_BYTE))
					bytes = CS_16_BITS;
				else if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_3_BYTE))
					bytes = CS_24_BITS;
				else
					bytes = CS_32_BITS;

				setListCount(bytes);
				break;

			case IDC_SIGNED:
			case IDC_UNSIGNED:
			case IDC_HEX:
				if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_UNSIGNED))
					val_type = 1;
				else if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_SIGNED))
					val_type = 2;
				else
					val_type = 3;

				updateCheatSearch();
				break;

			case IDC_C_ADD: {
			}
			break;

			case IDC_C_RESET:
				resetCheatSearch();
				setListCount(bytes);
				updateCheatSearch();
				return TRUE;

			case IDC_C_REFRESH:
				updateCheatSearch();
				return TRUE;

			case IDC_ENTERED:
			case IDC_ENTEREDADDRESS:
			case IDC_PREV:
				if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_ENTERED)) {
					use_entered = 1;
					EnableWindow(GetDlgItem(hwndDlg, IDC_VALUE_ENTER), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_ENTER_LABEL), TRUE);
				}
				else if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_ENTEREDADDRESS)) {
					use_entered = 2;
					EnableWindow(GetDlgItem(hwndDlg, IDC_VALUE_ENTER), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_ENTER_LABEL), TRUE);
				}
				else {
					use_entered = 0;
					EnableWindow(GetDlgItem(hwndDlg, IDC_VALUE_ENTER), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_ENTER_LABEL), FALSE);
				}
				return TRUE;

			case IDC_C_SEARCH: {
				val_type = 0;

				if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_LESS_THAN))
					comp_type = CS_LESS_THAN;
				else if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_GREATER_THAN))
					comp_type = CS_GREATER_THAN;
				else if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_LESS_THAN_EQUAL))
					comp_type = CS_LESS_THAN_OR_EQUAL;
				else if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_GREATER_THAN_EQUAL))
					comp_type = CS_GREATER_THAN_OR_EQUAL;
				else if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_EQUAL))
					comp_type = CS_EQUAL;
				else
					comp_type = CS_NOT_EQUAL;

				if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_UNSIGNED))
					val_type = 1;
				else if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_SIGNED))
					val_type = 2;
				else
					val_type = 3;

				if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_1_BYTE))
					bytes = CS_8_BITS;
				else if(BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_2_BYTE))
					bytes = CS_16_BITS;
				else if(BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_3_BYTE))
					bytes = CS_24_BITS;
				else
					bytes = CS_32_BITS;

				if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_ENTERED)
					|| BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_ENTEREDADDRESS)) {
					TCHAR buf[20];
					UINT32 value;

					GetDlgItemText(hwndDlg, IDC_VALUE_ENTER, buf, 20);
					if (use_entered == 2) {
						scanAddress(buf, &value);
						searchForAddress(comp_type, bytes, value, FALSE);
					} else {
						int ret;

						if (val_type == 1)
							ret = _stscanf(buf, _T("%ul"), &value);
						else if (val_type==2)
							ret = _stscanf(buf, _T("%d"), &value);
						else
							ret = _stscanf(buf, _T("%x"), &value);

						if (ret != 1 || !testRange(val_type, bytes, value)) {
							MessageBox(hwndDlg, FBALoadStringEx(IDS_CHEATSEARCH_INVALID), FBALoadStringEx(IDS_CHEATSEARCH_ERROR), MB_OK);
							return TRUE;
						}

						searchForValue(comp_type, bytes, value, (val_type == 2), FALSE);
					}
				} else {
					searchForChange(comp_type, bytes, (val_type == 2), FALSE);
				}

				setListCount(bytes);

				// if non-modal, update "Prev. Value" column after Search
				cheatSearchCopyRAM(cheatSearchInfo.CRAM);
				updateCheatSearch();
				return TRUE;
			}

			case IDOK:
			case IDCANCEL:
				DestroyWindow(hwndDlg);
				dialogDelete(IDD_CHEAT_SEARCH);
				cheatSearchDestroy();
				break;

			default:
				break;
		}

		return FALSE;
	}

#if 0
	if (msg == WM_ACTIVATE) {
		updateCheatSearch();
		return FALSE;
	}
#endif

	return FALSE;
}

#define USE_THREAD 0

#if USE_THREAD
int cheatSearchCreateDlg()
#else
int cheatSearchCreate()
#endif
{
#if !USE_THREAD
	if (!bDrvOkay || kNetGame) {
		return 1;
	}

	if (cheatSearchDlg) {
		// already open so just reactivate the window
		SetActiveWindow(cheatSearchDlg);
		return 0;
	}
#endif

	cheatSearchDlg = FBACreateDialog(IDD_CHEAT_SEARCH, hScrnWnd, (DLGPROC)cheatSearchDialogProc);
	if (cheatSearchDlg == NULL) {
		return 1;
	}
	dialogAdd(IDD_CHEAT_SEARCH, cheatSearchDlg);

	wndInMid(cheatSearchDlg, hScrnWnd);
	ShowWindow(cheatSearchDlg, SW_NORMAL);

	return 0;
}

static DWORD WINAPI doCheatSearch(LPVOID)
{
	MSG msg;
	BOOL bRet;

#if USE_THREAD
	cheatSearchCreateDlg();
#endif

	while (1) {
		bRet = GetMessage(&msg, NULL, 0, 0);

		if (bRet != 0 && bRet != -1) {
	    	// See if we need to end the thread
			if (msg.message == (WM_APP + 0)) {
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	DestroyWindow(cheatSearchDlg);

	return 0;
}

void cheatSearchDestroy()
{
#if !USE_THREAD
	return;
#endif

	if (cheatSearchThread == NULL) {
		return;
	}

	PostThreadMessage(cheatSearchThreadID, WM_APP + 0, 0, 0);

//	if (WaitForSingleObject(cheatSearchThread, 2000) != WAIT_OBJECT_0) {
//		TerminateThread(cheatSearchThread, 1);
//	}

	cheatSearchDlg = NULL;
	CloseHandle(cheatSearchThread);
	cheatSearchThread = NULL;
	cheatSearchThreadID = 0;
}

#if USE_THREAD
int cheatSearchCreate()
#else
int CheatSearchCreateDummy()
#endif
{
	if (!bDrvOkay || kNetGame) {
		return 1;
	}

	if (cheatSearchDlg) {
		// already open so just reactivate the window
		SetActiveWindow(cheatSearchDlg);
		return 0;
	}

	if (cheatSearchDlg || cheatSearchThread) {
		return 1;
	}

	cheatSearchThread = CreateThread(NULL, 0, doCheatSearch, NULL, THREAD_TERMINATE, &cheatSearchThreadID);
	return 0;
}

#endif
