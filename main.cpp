#include <iostream>
#include <string>
#include <vector>

int main()
{

  //Доступные методы:  
  //-run_Select_Folder_BlockThread  
  //-run_Select_Folder_NewThread  
  //-run_Select_OpenFiles_BlockThread  
  //-run_Select_OpenFiles_NewThread  
  //-run_Select_SaveFiles_BlockThread  
  //-run_Select_SaveFiles_NewThread  
  
 //Работает начиная с Windows Vista. Требуется C++11.  
 //Внутри используется подключение ShObjIdl.h - поэтому в свойствах проекта необходимо дополнительно подключить: shell32.lib и ole32.lib.

  WinApi__Save_Load_Dialog__class WinApi__Save_Load_Dialog__class_;



 //--------------------------------------------------------------------1-run_Select_Folder_BlockThread:Начало----------------------------------------------------------------------------
	 
 //Данная функция открывает диалоговое окно выбора папки/папок.
 //Данная функция является БЛОКИРУЮЩЕЙ, то есть после создания диалогового окна - поток его вызвавший блокируется до тех пор, пока Пользовтаель не выберет какую либо папку и нажмет "OK" или до тех пор пока ненажмет отмену, вообще до тех пор пока не закроет само диалогвоое окно.
	//Данная функция является Шаблонной: в качестве параметров шаблона принимает два типа: std::string или std::wstring. Более подробное описание читать чуть ниже.


	 WinApi__Save_Load_Dialog__class::result_flag result_flag_1;


	 //--------------------------------------------------------------------
	 //!!!ОТ ТИПА ПЕРЕМЕННОЙ ШАБЛОНА ЗАВИСИТ ТАК ЖЕ КОДИРОВКА, если тип указывается std::string, то кодировка должна быть UTF-8, если std::wstring, то кодировка должна быть UTF-16!!!
	 
	 const std::string PreviewPath_1 = u8"E:\\Загрузки\\";       //ЭТО ПЕРВЫЙ ШАБЛОННЫЙ ТИП: std::string или std::wstring. Здесь указывается первоначальный путь по которому откроется Диалогвоое окно. Если первоначальный путь не нужен, то просто передать пустую строку "", тогда первончальный путь откроется "по умолчанию".
	 
	 std::vector<std::string> vec_SelectFolders_1;               //ЭТО ВТОРОЙ ШАБЛОННЫЙ ТИП: std::string или std::wstring. Сюда функция занесет, выбранные Пользовталем папки в формате: "D:\My_Folder\sub_folder"
	
	 const WinApi__Save_Load_Dialog__class::flag FlagSelect_1 = WinApi__Save_Load_Dialog__class::flag::Multi_Select;      //One_Select - значит в Диалоговом окне можно будет выбрать только одну папку. Multi_Select - значит в Диалоговом окне можно будет выбрать больше одной папки.
	 //--------------------------------------------------------------------



	 //---------------------------------------------------------------------------------------------------------
	 result_flag_1 = WinApi__Save_Load_Dialog__class_.run_Select_Folder_BlockThread<std::string, std::string>(PreviewPath_1, vec_SelectFolders_1, FlagSelect_1);


	 if (result_flag_1 == WinApi__Save_Load_Dialog__class::result_flag::winapi_error)
	 {
		 std::cout << WinApi__Save_Load_Dialog__class_.get__ErrorName() << std::endl;

		 return -1;
	 } 
	 else
	 {
		 if (result_flag_1 == WinApi__Save_Load_Dialog__class::result_flag::Cancel)
		 {
			 std::cout << "Cancel" << std::endl;                                        //ЗНАЧИТ ПОЛЬЗОВАТЕЛЬ НИЧЕГО НЕ ВЫБРАЛ и просто закрыл Диалоговое окно.
		 }
		 else
		 {
			 if (result_flag_1 == WinApi__Save_Load_Dialog__class::result_flag::OK)
			 {
				 for (size_t i = 0; i < vec_SelectFolders_1.size(); i++)                          
				 {
					 std::cout << vec_SelectFolders_1[i] << std::endl;                     //Выводим папки, которые выбрал Пользователь.
					 //std::wcout << vec_SelectFolders_1[i] << std::endl;                  //Выводим папки, которые выбрал Пользователь.
				 }
			 }
		 }
	 }
	 //---------------------------------------------------------------------------------------------------------

	 
	//--------------------------------------------------------------------1-run_Select_Folder_BlockThread:Конец----------------------------------------------------------------------------




	
	



	//--------------------------------------------------------------------2-run_Select_Folder_NewThread:Начало----------------------------------------------------------------------------
	 /*
	//Данная функция открывает диалоговое окно выбора папки/папок.
	//Данная функция является НЕ БЛОКИРУЮЩЕЙ, сама функция внутри запускается в отдельном новом потоке.
	//Данная функция является Шаблонной: в качестве параметров шаблона принимает два типа: std::string или std::wstring. Более подробное описание читать чуть ниже.


	 WinApi__Save_Load_Dialog__class::result_flag result_flag_2;


	 //--------------------------------------------------------------------
	 //!!!ОТ ТИПА ПЕРЕМЕННОЙ ШАБЛОНА ЗАВИСИТ ТАК ЖЕ КОДИРОВКА, если тип указывается std::string, то кодировка должна быть UTF-8, если std::wstring, то кодировка должна быть UTF-16!!!

	 const std::string PreviewPath_2 = u8"D:\\Загрузки\\";       //ЭТО ПЕРВЫЙ ШАБЛОННЫЙ ТИП: std::string или std::wstring. Здесь указывается первоначальный путь по которому откроется Диалогвоое окно. Если первоначальный путь не нужен, то просто передать пустую строку "", тогда первончальный путь откроется "по умолчанию".

	 const WinApi__Save_Load_Dialog__class::flag FlagSelect_2 = WinApi__Save_Load_Dialog__class::flag::Multi_Select;      //One_Select - значит в Диалоговом окне можно будет выбрать только одну папку. Multi_Select - значит в Диалоговом окне можно будет выбрать больше одной папки.


	 std::function<void(const WinApi__Save_Load_Dialog__class::result_flag result_flag_, const std::string ErrorName, const std::vector<std::wstring>& vec_SelectFolders)> lambda_callback_2;  //Лямбда, которая вызовется функцией в случае ошибки, отмены или выбора папок в диалоговом окне.
		 
	 lambda_callback_2 = [](const WinApi__Save_Load_Dialog__class::result_flag result_flag_, const std::string ErrorName, const std::vector<std::wstring>& vec_SelectFolders)
		 {
			 //ТИП вектора "vec_SelectFolders" является ВТОРЫМ ШАБЛОНОМ ФУНКЦИИ: std::string или std::wstring.
			 //Данная лямбда запущена из потока отличного от того, в котором запускался метод "run_Select_OpenFiles_NewThread()"


			 if (result_flag_ == WinApi__Save_Load_Dialog__class::result_flag::winapi_error)
			 {
				 std::cout << ErrorName << std::endl;

				 return -1;
			 }
			 else
			 {
				 if (result_flag_ == WinApi__Save_Load_Dialog__class::result_flag::Cancel)
				 {
					 std::cout << "Cancel" << std::endl;                                        //ЗНАЧИТ ПОЛЬЗОВАТЕЛЬ НИЧЕГО НЕ ВЫБРАЛ и просто закрыл Диалоговое окно.
				 }
				 else
				 {
					 if (result_flag_ == WinApi__Save_Load_Dialog__class::result_flag::OK)
					 {
						 for (size_t i = 0; i < vec_SelectFolders.size(); i++)
						 {
							 //std::cout << vec_SelectFolders[i] << std::endl;                     //Выводим папки, которые выбрал Пользователь.
							 std::wcout << vec_SelectFolders[i] << std::endl;                  //Выводим папки, которые выбрал Пользователь.
						 }
					 }
				 }
			 }

			 //После завершения лямбды - созданный отдельный поток завершается.
		 };
	 //--------------------------------------------------------------------



	 //---------------------------------------------------------------------------------------------------------
	
	 WinApi__Save_Load_Dialog__class_.run_Select_Folder_NewThread<std::string, std::wstring>(PreviewPath_2, lambda_callback_2, FlagSelect_2); //Функция завершется сразу, вся обработка происходит в лямюде.

	 //---------------------------------------------------------------------------------------------------------


	 */
	//--------------------------------------------------------------------2-run_Select_Folder_NewThread:Конец----------------------------------------------------------------------------








	//--------------------------------------------------------------------3-run_Select_OpenFiles_BlockThread:Начало----------------------------------------------------------------------------
	 /*
	//Данная функция открывает диалоговое окно выбора файла/файлов.
	//Данная функция является БЛОКИРУЮЩЕЙ, то есть после создания диалогового окна - поток его вызвавший блокируется до тех пор, пока Пользовтаель не выберет какой либо файл и нажмет "OK" или до тех пор пока ненажмет отмену, вообще до тех пор пока не закроет само диалогвоое окно.
	//Данная функция является Шаблонной: в качестве параметров шаблона принимает два типа: std::string или std::wstring. Более подробное описание читать чуть ниже.


	 WinApi__Save_Load_Dialog__class::result_flag result_flag_3;


	 //--------------------------------------------------------------------
	 //!!!ОТ ТИПА ПЕРЕМЕННОЙ ШАБЛОНА ЗАВИСИТ ТАК ЖЕ КОДИРОВКА, если тип указывается std::string, то кодировка должна быть UTF-8, если std::wstring, то кодировка должна быть UTF-16!!!

	 const std::string PreviewPath_3 = u8"D:\\Загрузки\\";       //ЭТО ПЕРВЫЙ ШАБЛОННЫЙ ТИП: std::string или std::wstring. Здесь указывается первоначальный путь по которому откроется Диалогвоое окно. Если первоначальный путь не нужен, то просто передать пустую строку "", тогда первончальный путь откроется "по умолчанию".

	 std::vector<std::wstring> vec_SelectFolders_3;               //ЭТО ВТОРОЙ ШАБЛОННЫЙ ТИП: std::string или std::wstring. Сюда функция занесет, выбранные Пользовталем файлы в формате: "D:\My_Folder\sub_folder\my_file.jpg"
	

	 std::vector<COMDLG_FILTERSPEC>vec_FilterFileTypes_3;                  //Это вектор фильтрации типов файлов, то есть по каким типам файлов Пользователь в диалогвоом окне может отфильтровать содержимое. Тип расширения файла указанный в Первом элементе вектора - всегда будет фильтром по-умолчанию. ЕСЛИ  фильтры не нужны, то просто передать пустой вектор --> "{{}}"

	 vec_FilterFileTypes_3.resize(3);                                      //Пример заполнения вектора.
	 vec_FilterFileTypes_3[0].pszName = L"All Files (*.*)";                //Текстовое видимое описаение навания "фильтра"
	 vec_FilterFileTypes_3[0].pszSpec = L"*.*";                            //Тип расширения фильтруемых файлов.
	 vec_FilterFileTypes_3[1].pszName = L"Text Documents (*.txt)";
	 vec_FilterFileTypes_3[1].pszSpec = L"*.txt";
	 vec_FilterFileTypes_3[2].pszName = L"Image (*.jpg)";
	 vec_FilterFileTypes_3[2].pszSpec = L"*.jpg";


	 const WinApi__Save_Load_Dialog__class::flag FlagSelect_3 = WinApi__Save_Load_Dialog__class::flag::Multi_Select;      //One_Select - значит в Диалоговом окне можно будет выбрать только одну папку. Multi_Select - значит в Диалоговом окне можно будет выбрать больше одной папки.
	 //--------------------------------------------------------------------



	 //---------------------------------------------------------------------------------------------------------
	 result_flag_3 = WinApi__Save_Load_Dialog__class_.run_Select_OpenFiles_BlockThread<std::string, std::wstring>(PreviewPath_3, vec_SelectFolders_3, vec_FilterFileTypes_3, FlagSelect_3);


	 if (result_flag_3 == WinApi__Save_Load_Dialog__class::result_flag::winapi_error)
	 {
		 std::cout << WinApi__Save_Load_Dialog__class_.get__ErrorName() << std::endl;

		 return -1;
	 }
	 else
	 {
		 if (result_flag_3 == WinApi__Save_Load_Dialog__class::result_flag::Cancel)
		 {
			 std::cout << "Cancel" << std::endl;                                        //ЗНАЧИТ ПОЛЬЗОВАТЕЛЬ НИЧЕГО НЕ ВЫБРАЛ и просто закрыл Диалоговое окно.
		 }
		 else
		 {
			 if (result_flag_3 == WinApi__Save_Load_Dialog__class::result_flag::OK)
			 {
				 for (size_t i = 0; i < vec_SelectFolders_3.size(); i++)
				 {
					 //std::cout << vec_SelectFolders_3[i] << std::endl;                     //Выводим файлы, которые выбрал Пользователь.
					 std::wcout << vec_SelectFolders_3[i] << std::endl;                  //Выводим файлы, которые выбрал Пользователь.
				 }
			 }
		 }
	 }
	 //---------------------------------------------------------------------------------------------------------

	 */
	 //--------------------------------------------------------------------3-run_Select_OpenFiles_BlockThread:Конец----------------------------------------------------------------------------








	 //--------------------------------------------------------------------4-run_Select_OpenFiles_NewThread:Начало----------------------------------------------------------------------------
	  /*
	 //Данная функция открывает диалоговое окно выбора файла/файлов.
	 //Данная функция является НЕ БЛОКИРУЮЩЕЙ, сама функция внутри запускается в отдельном новом потоке.
	 //Данная функция является Шаблонной: в качестве параметров шаблона принимает два типа: std::string или std::wstring. Более подробное описание читать чуть ниже.



	  //--------------------------------------------------------------------
	  //!!!ОТ ТИПА ПЕРЕМЕННОЙ ШАБЛОНА ЗАВИСИТ ТАК ЖЕ КОДИРОВКА, если тип указывается std::string, то кодировка должна быть UTF-8, если std::wstring, то кодировка должна быть UTF-16!!!

	  const std::wstring PreviewPath_4 = L"D:\\Загрузки\\";       //ЭТО ПЕРВЫЙ ШАБЛОННЫЙ ТИП: std::string или std::wstring. Здесь указывается первоначальный путь по которому откроется Диалогвоое окно. Если первоначальный путь не нужен, то просто передать пустую строку "", тогда первончальный путь откроется "по умолчанию".


	  std::vector<COMDLG_FILTERSPEC>vec_FilterFileTypes_4;                  //Это вектор фильтрации типов файлов, то есть по каким типам файлов Пользователь в диалогвоом окне может отфильтровать содержимое. Тип расширения файла указанный в Первом элементе вектора - всегда будет фильтром по-умолчанию. ЕСЛИ  фильтры не нужны, то просто передать пустой вектор --> "{{}}"
	  vec_FilterFileTypes_4.resize(3);                                      //Пример заполнения вектора.
	  vec_FilterFileTypes_4[0].pszName = L"All Files (*.*)";                //Текстовое видимое описаение навания "фильтра"
	  vec_FilterFileTypes_4[0].pszSpec = L"*.*";                            //Тип расширения фильтруемых файлов.
	  vec_FilterFileTypes_4[1].pszName = L"Text Documents (*.txt)";
	  vec_FilterFileTypes_4[1].pszSpec = L"*.txt";
	  vec_FilterFileTypes_4[2].pszName = L"Image (*.jpg)";
	  vec_FilterFileTypes_4[2].pszSpec = L"*.jpg";


	  const WinApi__Save_Load_Dialog__class::flag FlagSelect_4 = WinApi__Save_Load_Dialog__class::flag::Multi_Select;      //One_Select - значит в Диалоговом окне можно будет выбрать только одну папку. Multi_Select - значит в Диалоговом окне можно будет выбрать больше одной папки.



	  std::function<void(const WinApi__Save_Load_Dialog__class::result_flag result_flag_, const std::string ErrorName, const std::vector<std::wstring>& vec_SelectFolders)> lambda_callback_4;  //Лямбда, которая вызовется функцией в случае ошибки, отмены или выбора файлов в диалоговом окне.

	  lambda_callback_4 = [](const WinApi__Save_Load_Dialog__class::result_flag result_flag_, const std::string ErrorName, const std::vector<std::wstring>& vec_SelectFolders)
		  {
			  //ТИП вектора "vec_SelectFolders" является ВТОРЫМ ШАБЛОНОМ ФУНКЦИИ: std::string или std::wstring.
			  //Данная лямбда запущена из потока отличного от того, в котором запускался метод "run_Select_OpenFiles_NewThread()"

			  if (result_flag_ == WinApi__Save_Load_Dialog__class::result_flag::winapi_error)
			  {
				  std::cout << ErrorName << std::endl;

				  return -1;
			  }
			  else
			  {
				  if (result_flag_ == WinApi__Save_Load_Dialog__class::result_flag::Cancel)
				  {
					  std::cout << "Cancel" << std::endl;                                        //ЗНАЧИТ ПОЛЬЗОВАТЕЛЬ НИЧЕГО НЕ ВЫБРАЛ и просто закрыл Диалоговое окно.
				  }
				  else
				  {
					  if (result_flag_ == WinApi__Save_Load_Dialog__class::result_flag::OK)
					  {
						  for (size_t i = 0; i < vec_SelectFolders.size(); i++)
						  {
							  //std::cout << vec_SelectFolders[i] << std::endl;                     //Выводим файлы, которые выбрал Пользователь.
							  std::wcout << vec_SelectFolders[i] << std::endl;                      //Выводим файлы, которые выбрал Пользователь.
						  }
					  }
				  }
			  }

			  //После завершения лямбды - созданный отдельный поток завершается.
		  };
	  //--------------------------------------------------------------------



	  //---------------------------------------------------------------------------------------------------------

	    WinApi__Save_Load_Dialog__class_.run_Select_OpenFiles_NewThread<std::wstring, std::wstring>(PreviewPath_4, vec_FilterFileTypes_4, lambda_callback_4, FlagSelect_4); //Функция завершется сразу, вся обработка происходит в лямюде.

	  //---------------------------------------------------------------------------------------------------------

	  
	  */
	  //--------------------------------------------------------------------4-run_Select_OpenFiles_NewThread:Конец----------------------------------------------------------------------------









	//--------------------------------------------------------------------5-run_Select_SaveFile_BlockThread:Начало----------------------------------------------------------------------------
	 /*
	//Данная функция открывает диалоговое окно выбора файла/файлов.
	//Данная функция является БЛОКИРУЮЩЕЙ, то есть после создания диалогового окна - поток его вызвавший блокируется до тех пор, пока Пользовтаель не выберет какой либо файл и нажмет "OK" или до тех пор пока ненажмет отмену, вообще до тех пор пока не закроет само диалогвоое окно.
	//Данная функция является Шаблонной: в качестве параметров шаблона принимает два типа: std::string или std::wstring. Более подробное описание читать чуть ниже.


	 WinApi__Save_Load_Dialog__class::result_flag result_flag_5;


	 //--------------------------------------------------------------------
	 //!!!ОТ ТИПА ПЕРЕМЕННОЙ ШАБЛОНА ЗАВИСИТ ТАК ЖЕ КОДИРОВКА, если тип указывается std::string, то кодировка должна быть UTF-8, если std::wstring, то кодировка должна быть UTF-16!!!

	 const std::string PreviewPath_5 = u8"D:\\Загрузки\\";                  //ЭТО ПЕРВЫЙ ШАБЛОННЫЙ ТИП: std::string или std::wstring. Здесь указывается первоначальный путь по которому откроется Диалогвоое окно. Если первоначальный путь не нужен, то просто передать пустую строку "", тогда первончальный путь откроется "по умолчанию".

	 std::wstring selected_file_5;                                         //ЭТО ВТОРОЙ ШАБЛОННЫЙ ТИП: std::string или std::wstring. Сюда функция занесет, выбранные Пользовталем файлы в формате: "D:\My_Folder\sub_folder\my_file.jpg"
	 

	 std::vector<COMDLG_FILTERSPEC>vec_FilterFileTypes_5;                  //Это вектор фильтрации типов файлов, то есть по каким типам файлов Пользователь в диалогвоом окне может отфильтровать содержимое. Тип расширения файла указанный в Первом элементе вектора - всегда будет фильтром по-умолчанию. ЕСЛИ  фильтры не нужны, то просто передать пустой вектор --> "{{}}"
	 
	 vec_FilterFileTypes_5.resize(3);                                      //Пример заполнения вектора.
	 vec_FilterFileTypes_5[0].pszName = L"All Files (*.*)";                //Текстовое видимое описаение навания "фильтра"
	 vec_FilterFileTypes_5[0].pszSpec = L"*.*";                            //Тип расширения фильтруемых файлов.
	 vec_FilterFileTypes_5[1].pszName = L"Text Documents (*.txt)";
	 vec_FilterFileTypes_5[1].pszSpec = L"*.txt";
	 vec_FilterFileTypes_5[2].pszName = L"Image (*.jpg)";
	 vec_FilterFileTypes_5[2].pszSpec = L"*.jpg";
	 //--------------------------------------------------------------------



	 //---------------------------------------------------------------------------------------------------------
	 result_flag_5 = WinApi__Save_Load_Dialog__class_.run_Select_SaveFile_BlockThread<std::string, std::wstring>(PreviewPath_5, selected_file_5, vec_FilterFileTypes_5);


	 if (result_flag_5 == WinApi__Save_Load_Dialog__class::result_flag::winapi_error)
	 {
		 std::cout << WinApi__Save_Load_Dialog__class_.get__ErrorName() << std::endl;

		 return -1;
	 }
	 else
	 {
		 if (result_flag_5 == WinApi__Save_Load_Dialog__class::result_flag::Cancel)
		 {
			 std::cout << "Cancel" << std::endl;                                        //ЗНАЧИТ ПОЛЬЗОВАТЕЛЬ НИЧЕГО НЕ ВЫБРАЛ и просто закрыл Диалоговое окно.
		 }
		 else
		 {
			 if (result_flag_5 == WinApi__Save_Load_Dialog__class::result_flag::OK)
			 {
				//std::cout << selected_file_5 << std::endl;                 //Выводим выбранный файл для сохранения, который выбрал Пользователь.
				std::wcout << selected_file_5 << std::endl;                  //Выводим выбранный файл для сохранения, который выбрал Пользователь.
			 }
		 }
	 }
	 //---------------------------------------------------------------------------------------------------------
	 
	 */
	 //--------------------------------------------------------------------5-run_Select_SaveFile_BlockThread:Конец----------------------------------------------------------------------------






	//--------------------------------------------------------------------6-run_Select_SaveFile_NewThread:Начало----------------------------------------------------------------------------
  /*
	//Данная функция открывает диалоговое окно выбора файла/файлов.
	//Данная функция является НЕ БЛОКИРУЮЩЕЙ, сама функция внутри запускается в отдельном новом потоке.
	//Данная функция является Шаблонной: в качестве параметров шаблона принимает два типа: std::string или std::wstring. Более подробное описание читать чуть ниже.


	 //--------------------------------------------------------------------
	 //!!!ОТ ТИПА ПЕРЕМЕННОЙ ШАБЛОНА ЗАВИСИТ ТАК ЖЕ КОДИРОВКА, если тип указывается std::string, то кодировка должна быть UTF-8, если std::wstring, то кодировка должна быть UTF-16!!!

	 const std::string PreviewPath_6 = u8"D:\\Загрузки\\";                  //ЭТО ПЕРВЫЙ ШАБЛОННЫЙ ТИП: std::string или std::wstring. Здесь указывается первоначальный путь по которому откроется Диалогвоое окно. Если первоначальный путь не нужен, то просто передать пустую строку "", тогда первончальный путь откроется "по умолчанию".

	 std::vector<COMDLG_FILTERSPEC>vec_FilterFileTypes_6;                  //Это вектор фильтрации типов файлов, то есть по каким типам файлов Пользователь в диалогвоом окне может отфильтровать содержимое. Тип расширения файла указанный в Первом элементе вектора - всегда будет фильтром по-умолчанию. ЕСЛИ  фильтры не нужны, то просто передать пустой вектор --> "{{}}"
	 vec_FilterFileTypes_6.resize(3);                                      //Пример заполнения вектора.
	 vec_FilterFileTypes_6[0].pszName = L"All Files (*.*)";                //Текстовое видимое описаение навания "фильтра"
	 vec_FilterFileTypes_6[0].pszSpec = L"*.*";                            //Тип расширения фильтруемых файлов.
	 vec_FilterFileTypes_6[1].pszName = L"Text Documents (*.txt)";
	 vec_FilterFileTypes_6[1].pszSpec = L"*.txt";
	 vec_FilterFileTypes_6[2].pszName = L"Image (*.jpg)";
	 vec_FilterFileTypes_6[2].pszSpec = L"*.jpg";


	 std::function<void(const WinApi__Save_Load_Dialog__class::result_flag result_flag_, const std::string ErrorName, const std::wstring& SelectSaveFile)> lambda_callback_6;  //Лямбда, которая вызовется функцией в случае ошибки, отмены или выбора файла для созранения в диалоговом окне.

	 lambda_callback_6 = [](const WinApi__Save_Load_Dialog__class::result_flag result_flag_, const std::string ErrorName, const std::wstring& SelectSaveFile)
		 {
			 //ТИП строки "SelectSaveFile" является ВТОРЫМ ШАБЛОНОМ ФУНКЦИИ: std::string или std::wstring.
			 //Данная лямбда запущена из потока отличного от того, в котором запускался метод "run_Select_SaveFile_NewThread()"

			 if (result_flag_ == WinApi__Save_Load_Dialog__class::result_flag::winapi_error)
			 {
				 std::cout << ErrorName << std::endl;

				 return -1;
			 }
			 else
			 {
				 if (result_flag_ == WinApi__Save_Load_Dialog__class::result_flag::Cancel)
				 {
					 std::cout << "Cancel" << std::endl;                                        //ЗНАЧИТ ПОЛЬЗОВАТЕЛЬ НИЧЕГО НЕ ВЫБРАЛ и просто закрыл Диалоговое окно.
				 }
				 else
				 {
					 if (result_flag_ == WinApi__Save_Load_Dialog__class::result_flag::OK)
					 {
						 //std::cout << SelectSaveFile << std::endl;                     //Выводим файлы, которые выбрал Пользователь.
						 std::wcout << SelectSaveFile << std::endl;                      //Выводим файлы, которые выбрал Пользователь. 
					 }
				 }
			 }

			 //После завершения лямбды - созданный отдельный поток завершается.
		 };
	 //--------------------------------------------------------------------



	 //---------------------------------------------------------------------------------------------------------
	 
	 WinApi__Save_Load_Dialog__class_.run_Select_SaveFile_NewThread<std::string, std::wstring>(PreviewPath_6, vec_FilterFileTypes_6, lambda_callback_6);   //Функция завершется сразу, вся обработка происходит в лямюде.

	 //---------------------------------------------------------------------------------------------------------

	 */
	 //--------------------------------------------------------------------6-run_Select_SaveFile_NewThread:Конец----------------------------------------------------------------------------



}
