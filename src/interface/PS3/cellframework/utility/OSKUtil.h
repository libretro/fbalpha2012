#ifndef __OSKUTIL_H__
#define __OSKUTIL_H__

#include <sysutil/sysutil_oskdialog.h>
#include <sysutil/sysutil_common.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>

class OSKUtil
{
   public:
      OSKUtil();
      OSKUtil(std::string& msg, std::string& init);
      ~OSKUtil();

      //virtual uint64_t getString(std::string& out) = 0;
      uint64_t getString(std::string& out);
      const char * OutputString();
      bool Start(const wchar_t* msg, const wchar_t* init);
      bool Abort();
      void Stop();
      void Close();
   protected:
      void str_to_utf16(uint16_t*& buf, const std::string& str);
   private:
      uint32_t mFlags;
      //uint16_t result_text_buffer[CELL_OSKDIALOG_STRING_SIZE + 1];
      wchar_t result_text_buffer[CELL_OSKDIALOG_STRING_SIZE + 1];
      uint16_t* msg_buf;
      uint16_t* init_buf;
      char result_text_buffer_char[256 + 1];
      std::string m_msg;
      std::string m_init;
      sys_memory_container_t containerid;
      CellOskDialogPoint pos;
      CellOskDialogInputFieldInfo inputFieldInfo;
      CellOskDialogCallbackReturnParam outputInfo;
      // Onscreen keyboard dialog utility activation parameters
      CellOskDialogParam dialogParam;

      //Functions
      void CreateActivationParameters();
      bool EnableKeyLayout();
};

enum{
	MODE_IDLE = 0,
	MODE_OPEN,
	MODE_RUNNING,
	MODE_CLOSE,
	MODE_ENTERED,
	MODE_CANCELED,
	SET_ABOR_TIMER,
	CHANGE_PANEL_MODE,
	MODE_EXIT_OSK,
	START_DIALOG_TYPE,
	START_SEPARATE_TYPE_1,
	START_SEPARATE_TYPE_2,
};

#endif
