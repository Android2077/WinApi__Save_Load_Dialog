#pragma once

#include <windows.h>
#include <ShObjIdl.h>

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <type_traits>   //для проверки шаблонного типа через is_same


class WinApi__Save_Load_Dialog__class
{

    //https://learn.microsoft.com/en-us/windows/win32/shell/common-file-dialog



public:


    enum class flag
    {
        One_Select = 1,
        Multi_Select = 2,
    };


    enum class result_flag
    {
        OK = 1,
        Cancel = 2,
        winapi_error = 3,
        bad_alloc = 4,
    };



public:


    template<typename string_wstring_T1, typename string_wstring_T2> const result_flag run_Select_Folder_BlockThread(const string_wstring_T1& Preview_Path, std::vector<string_wstring_T2>& vec_SelectFolders, const flag NumSelect_flag)
    {


        HRESULT HRESULT_;


        //-----------------------------------------------0-Инициализируем бибилотеку COM:Начало----------------------------------------------

        HRESULT_ = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);       //Инициализируем бибилотеку COM: https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-coinitializeex.  MSDN рекумендует установить флаг "COINIT_DISABLE_OLE1DDE" для того, чтобы какая то там устарвешая модель не использовалась, вот тут: : https://learn.microsoft.com/en-us/windows/win32/learnwin32/initializing-the-com-library

        if (HRESULT_ != S_OK && HRESULT_ != S_FALSE)
        {
            //Значит HRESULT_ == RPC_E_CHANGED_MODE: Предыдущий вызов CoInitializeEx указал несовместимую модель параллелизма для этого потока. Это также может указывать на то, что произошло изменение с нейтрального многопоточного режима на однопоточный.

            error = "CoInitializeEx: " + get__ErrorText(HRESULT_);

            return result_flag::winapi_error;
        }

        //-----------------------------------------------0-Инициализируем бибилотеку COM:Конец----------------------------------------------



        //--------------------------------------------1-Создаем обьект Диалогвоого окна:Начало-----------------------------------------------------

        IFileOpenDialog* IFileOpenDialog_p = NULL;

        HRESULT_ = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_IFileOpenDialog, (LPVOID*)&IFileOpenDialog_p);    //Создаем экземпляр диалогового окна. Первый параметр "CLSID_FileOpenDialog", это указание функции, что в обьект указателя на "диалоговок окна" нужно поместить именно указатель на обьект "диалогвого окно". Второй и треьтий парметры - хер знает, что это такое. В примерах они указаны - ну и хорошо. "IID_IFileDialog" - указывает какие методы будут доступны в "pFileDialog", 

        if (HRESULT_ != S_OK)
        {
            CoUninitialize();  //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

            //-----------------------------------------------------------------
            error = "CoCreateInstance: " + get__ErrorText(HRESULT_);

            return  result_flag::winapi_error;
            //-----------------------------------------------------------------
        }

        //IFileOpenDialog_p - теперь тут указатель на обьект класса "IFileOpenDialog", у которого есть свои два метода(https://learn.microsoft.com/ru-ru/windows/win32/api/shobjidl_core/nn-shobjidl_core-ifileopendialog) и все методы унаследованные от IFileDialog(https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-ifiledialog)

        //--------------------------------------------1-Создаем обьект Диалогвоого окна:Конец-----------------------------------------------------




         //-------------------------------------------------------2-Устанавливаем необходимые опции для Диалогового окна:Начало-------------------------------------------------------------------------------

        DWORD dwFlags; IFileOpenDialog_p->GetOptions(&dwFlags);         //Получаем опции, который установлены по умолчанию для "IFileOpenDialog", чтобы потом убрать или доабвить к ним другие опции.

        //Список опций, которые принимает "SetOptions" можно посомтреть тут:  https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/ne-shobjidl_core-_fileopendialogoptions

        //В данном случае используем только следующие: 

        //-FOS_PICKFOLDERS:        Представьте открытое диалоговое окно, предлагающее выбор папок, а не файлов.
        //-FOS_ALLOWMULTISELECT:   Позволяет пользователю выбрать несколько элементов в диалоговом окне открытия.Обратите внимание, что при установке этого флага для получения этих элементов необходимо использовать интерфейс IFileOpenDialog.
        //-FOS_NODEREFERENCELINKS: Ярлыки не следует рассматривать как целевые элементы. Это позволяет приложению открывать файл .lnk, а не то, на что указывает ярлык.



        //`````````````````````````````````````````````````````
        if (NumSelect_flag == flag::One_Select)
        {
            HRESULT_ = IFileOpenDialog_p->SetOptions(dwFlags | FOS_PICKFOLDERS  | FOS_NODEREFERENCELINKS);
        }
        else if (NumSelect_flag == flag::Multi_Select)
        {
            HRESULT_ = IFileOpenDialog_p->SetOptions(dwFlags | FOS_PICKFOLDERS | FOS_ALLOWMULTISELECT | FOS_NODEREFERENCELINKS);
        }
        //`````````````````````````````````````````````````````
       

        if (HRESULT_ != S_OK)
        {
            error = "SetOptions: " + get__ErrorText(HRESULT_);

            return  result_flag::winapi_error;
        }

        //-------------------------------------------------------2-Устанавливаем необходимые опции для Диалогового окна:Конец-------------------------------------------------------------------------------




        //--------------------------------------------------------------3-Устанавливаем отображение диалогового окна:Конец----------------------------------------------------------------------------


        //--------------------------------------Если Пользовтаель указал конкретный начальный путь на котором должно октрытся диалоговое окно:-------------------
        const result_flag result_flag_ = PreviewOpenPath_handler<string_wstring_T1>(IFileOpenDialog_p, Preview_Path);

        if (result_flag_ != result_flag::OK) { error = "PreviewOpenPath_handler: " + error;  return result_flag_; }
        //-------------------------------------------------------------------------------------------------------------------------------------------------------



        HRESULT_ = IFileOpenDialog_p->Show(NULL);     //Через наследование от IModalWindow, вызываем "show": https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-imodalwindow-show

        if (HRESULT_ != S_OK && HRESULT_ != HRESULT_FROM_WIN32(ERROR_CANCELLED))
        {
            //**************************************************
            CoUninitialize();           //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

            IFileOpenDialog_p->Release(); //Освобождаем память.
            //**************************************************

            error = "Show: " + get__ErrorText(HRESULT_);

            return  result_flag::winapi_error;
        }
        else
        {
            if (HRESULT_ == HRESULT_FROM_WIN32(ERROR_CANCELLED))
            {
                //Значит Пользователь закрыл окно - так ничего и не выбрав.

                //**************************************************
                CoUninitialize();           //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

                IFileOpenDialog_p->Release(); //Освобождаем память.
                //**************************************************

                return result_flag::Cancel;
            }
        }

        //--------------------------------------------------------------3-Устанавливаем отображение диалогового окна:Конец----------------------------------------------------------------------------



        //Если код дошел до сюда, значит Пользователь уже выбрал какую то папку и нажал "Открыть".


        //----------------------------------------------------------------4-Получим результат выбора Пользовтаеля:Начало--------------------------------------------------------------------------

        //Вызываем метод "GetResults". "GetResults" приниает, адрес указателя на класс или стурктуры "IShellItemArray" и выделяет память под него: https://learn.microsoft.com/ru-ru/windows/win32/api/shobjidl_core/nn-shobjidl_core-ifileopendialog

        IShellItemArray* Shell_Array;

        HRESULT_ = IFileOpenDialog_p->GetResults(&Shell_Array);

        if (HRESULT_ != S_OK)
        {
            //**************************************************
            CoUninitialize();            //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

            IFileOpenDialog_p->Release(); //Освобождаем память.
            //**************************************************


            error = "GetResult: " + get__ErrorText(HRESULT_);

            return  result_flag::winapi_error;
        }

        //----------------------------------------------------------------4-Получим результат выбора Пользовтаеля:Конец--------------------------------------------------------------------------





        //-------------------------------------------------------------5-Получим кол-во выбранных Пользователем элеметов:Начало-------------------------------------------------------------------

        DWORD NumCount;

        HRESULT_ = Shell_Array->GetCount(&NumCount);   //Полуxим кол-во выбранных Пользователем элеметов.  //https://learn.microsoft.com/ru-ru/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishellitemarray-getcount

        if (HRESULT_ != S_OK)
        {
            //**************************************************
            CoUninitialize();            //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

            IFileOpenDialog_p->Release(); //Освобождаем память.

            Shell_Array->Release();     //Освобождаем память.
            //**************************************************

            error = "GetCount: " + get__ErrorText(HRESULT_);

            return  result_flag::winapi_error;
        }

        //NumCount - теперь тут кол-во выбранных Пользователем элеметов.

    //-------------------------------------------------------------5-Получим кол-во выбранных Пользователем элеметов:Конец-------------------------------------------------------------------




    //-------------------------------------------------------------5-Получим текст выбранных Пользователем элеметов:Начало-------------------------------------------------------------------

        vec_SelectFolders.resize(NumCount);


        for (DWORD i = 0; i < NumCount; i++)
        {
            IShellItem* IShellItem_p;

            HRESULT_ = Shell_Array->GetItemAt(i, &IShellItem_p);             //https://learn.microsoft.com/ru-ru/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishellitemarray-getitemat

            if (HRESULT_ != S_OK) { CoUninitialize(); IFileOpenDialog_p->Release(); Shell_Array->Release(); error = "GetItemAt: " + get__ErrorText(HRESULT_); return  result_flag::winapi_error; }


            //---------------------Получаем имя элемента:-------------------
            PWSTR PWSTR_ElementName;

            HRESULT_ = IShellItem_p->GetDisplayName(SIGDN_FILESYSPATH, &PWSTR_ElementName);                  //https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishellitem-getdisplayname  и https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/ne-shobjidl_core-sigdn

            if (HRESULT_ != S_OK) { CoUninitialize(); IFileOpenDialog_p->Release(); Shell_Array->Release(); error = "GetItemAt: " + get__ErrorText(HRESULT_); IShellItem_p->Release(); return  result_flag::winapi_error; }

            //PWSTR_ElementName  - сюда функция выделила память и поместила текст пути выбранной паки. Кодирова UTF-16 wstring.

            add_folder_to_Vector<string_wstring_T2>(i, PWSTR_ElementName, vec_SelectFolders);      //Добавим выбранную папку в вектор результатов.

            CoTaskMemFree(PWSTR_ElementName); //Вызывающая сторона обязана освободить строку, на которую указывает ppszName, когда она больше не нужна. Вызовите CoTaskMemFree, чтобы освободить память.
            //----------------------------------------


            IShellItem_p->Release();   //Освобождаем память, она уже больше не нужна - мы забарли уже все что хотели.
        }

        //-------------------------------------------------------------5-Получим текст выбранных Пользователем элеметов:Конец-------------------------------------------------------------------




        //Собсвенно все, освобождаем, ВСЕ задейстованеые ресурсы.

        //------------------------------------------------------

        CoUninitialize();             //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

        IFileOpenDialog_p->Release();   //Освобождаем память.

        Shell_Array->Release();       //Освобождаем память.

        //------------------------------------------------------


        return result_flag::OK;

    }

    template<typename string_wstring_T1, typename string_wstring_T2> void run_Select_Folder_NewThread(const string_wstring_T1& Preview_Path, std::function<void(const result_flag result_flag_, const std::string ErrorName, const  std::vector<string_wstring_T2>& vec_SelectFolders)>func_, const flag NumSelect_flag)
    {
        std::thread New_thread(&WinApi__Save_Load_Dialog__class::run_Select_Folder_NewThread_<string_wstring_T1, string_wstring_T2>, this, Preview_Path, func_, NumSelect_flag);

        New_thread.detach();
    }


    template<typename string_wstring_T1, typename string_wstring_T2> const result_flag run_Select_OpenFiles_BlockThread(const string_wstring_T1& Preview_Path, std::vector<string_wstring_T2>& vec_SelectFiles, const std::vector<COMDLG_FILTERSPEC>& vec_FilterFileTypes, const flag NumSelect_flag)
    {


        HRESULT HRESULT_;


        //-----------------------------------------------0-Инициализируем бибилотеку COM:Начало----------------------------------------------

        HRESULT_ = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);       //Инициализируем бибилотеку COM: https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-coinitializeex.  MSDN рекумендует установить флаг "COINIT_DISABLE_OLE1DDE" для того, чтобы какая то там устарвешая модель не использовалась, вот тут: : https://learn.microsoft.com/en-us/windows/win32/learnwin32/initializing-the-com-library

        if (HRESULT_ != S_OK && HRESULT_ != S_FALSE)
        {
            //Значит HRESULT_ == RPC_E_CHANGED_MODE: Предыдущий вызов CoInitializeEx указал несовместимую модель параллелизма для этого потока. Это также может указывать на то, что произошло изменение с нейтрального многопоточного режима на однопоточный.

            error = "CoInitializeEx: " + get__ErrorText(HRESULT_);

            return result_flag::winapi_error;
        }

        //-----------------------------------------------0-Инициализируем бибилотеку COM:Конец----------------------------------------------



        //--------------------------------------------1-Создаем обьект Диалогвоого окна:Начало-----------------------------------------------------

        IFileOpenDialog* IFileOpenDialog_p = NULL;

        HRESULT_ = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_IFileOpenDialog, (LPVOID*)&IFileOpenDialog_p);    //Создаем экземпляр диалогового окна. Первый параметр "CLSID_FileOpenDialog", это указание функции, что в обьект указателя на "диалоговок окна" нужно поместить именно указатель на обьект "диалогвого окно". Второй и треьтий парметры - хер знает, что это такое. В примерах они указаны - ну и хорошо. "IID_IFileDialog" - указывает какие методы будут доступны в "pFileDialog", 

        if (HRESULT_ != S_OK)
        {
            CoUninitialize();  //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

            //-----------------------------------------------------------------
            error = "CoCreateInstance: " + get__ErrorText(HRESULT_);

            return  result_flag::winapi_error;
            //-----------------------------------------------------------------
        }

        //IFileOpenDialog_p - теперь тут указатель на обьект класса "IFileOpenDialog", у которого есть свои два метода(https://learn.microsoft.com/ru-ru/windows/win32/api/shobjidl_core/nn-shobjidl_core-ifileopendialog) и все методы унаследованные от IFileDialog(https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-ifiledialog)

        //--------------------------------------------1-Создаем обьект Диалогвоого окна:Конец-----------------------------------------------------




         //-------------------------------------------------------2-Устанавливаем необходимые опции для Диалогового окна:Начало-------------------------------------------------------------------------------

        DWORD dwFlags; IFileOpenDialog_p->GetOptions(&dwFlags);         //Получаем опции, который установлены по умолчанию для "IFileOpenDialog", чтобы потом убрать или доабвить к ним другие опции.

        //Список опций, которые принимает "SetOptions" можно посомтреть тут:  https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/ne-shobjidl_core-_fileopendialogoptions

        //В данном случае используем только следующие: 

        //-FOS_ALLOWMULTISELECT:   Позволяет пользователю выбрать несколько элементов в диалоговом окне открытия.Обратите внимание, что при установке этого флага для получения этих элементов необходимо использовать интерфейс IFileOpenDialog. (с)
        //-FOS_NODEREFERENCELINKS: Ярлыки не следует рассматривать как целевые элементы. Это позволяет приложению открывать файл .lnk, а не то, на что указывает ярлык. (с)


        //`````````````````````````````````````````````````````
        if (NumSelect_flag == flag::One_Select)
        {
            HRESULT_ = IFileOpenDialog_p->SetOptions(dwFlags  | FOS_NODEREFERENCELINKS);
        }
        else if (NumSelect_flag == flag::Multi_Select)
        {
            HRESULT_ = IFileOpenDialog_p->SetOptions(dwFlags | FOS_ALLOWMULTISELECT | FOS_NODEREFERENCELINKS);
        }
        //`````````````````````````````````````````````````````


        if (HRESULT_ != S_OK)
        {
            error = "SetOptions: " + get__ErrorText(HRESULT_);

            return  result_flag::winapi_error;
        }

        //-------------------------------------------------------2-Устанавливаем необходимые опции для Диалогового окна:Конец-------------------------------------------------------------------------------







        //--------------------------------------------------------------Устанавливаем Пользовательский список фильтров по типу файла:Начало----------------------------------------------------------------------------

        IFileOpenDialog_p->SetFileTypes(vec_FilterFileTypes.size(), vec_FilterFileTypes.data());


        IFileOpenDialog_p->SetFileTypeIndex(1); //  Устанавливаем фильтр по умолчанию, который будет действовать при создании диалоогового окна. Ставим первый элеммент вектора.

        //--------------------------------------------------------------Устанавливаем Пользовательский список фильтров по типу файла:Конец----------------------------------------------------------------------------








        //--------------------------------------------------------------3-Устанавливаем отображение диалогового окна:Конец----------------------------------------------------------------------------


        //--------------------------------------Если Пользовтаель указал конкретный начальный путь на котором должно октрытся диалоговое окно:-------------------
        const result_flag result_flag_ = PreviewOpenPath_handler<string_wstring_T1>(IFileOpenDialog_p, Preview_Path);

        if (result_flag_ != result_flag::OK) { error = "PreviewOpenPath_handler: " + error;  return result_flag_; }
        //-------------------------------------------------------------------------------------------------------------------------------------------------------



        HRESULT_ = IFileOpenDialog_p->Show(NULL);     //Через наследование от IModalWindow, вызываем "show": https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-imodalwindow-show

        if (HRESULT_ != S_OK && HRESULT_ != HRESULT_FROM_WIN32(ERROR_CANCELLED))
        {
            //**************************************************
            CoUninitialize();           //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

            IFileOpenDialog_p->Release(); //Освобождаем память.
            //**************************************************

            error = "Show: " + get__ErrorText(HRESULT_);

            return  result_flag::winapi_error;
        }
        else
        {
            if (HRESULT_ == HRESULT_FROM_WIN32(ERROR_CANCELLED))
            {
                //Значит Пользователь закрыл окно - так ничего и не выбрав.

                //**************************************************
                CoUninitialize();             //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

                IFileOpenDialog_p->Release(); //Освобождаем память.
                //**************************************************

                return result_flag::Cancel;
            }
        }

        //--------------------------------------------------------------3-Устанавливаем отображение диалогового окна:Конец----------------------------------------------------------------------------



        //Если код дошел до сюда, значит Пользователь уже выбрал какую то папку и нажал "Открыть".


        //----------------------------------------------------------------4-Получим результат выбора Пользовтаеля:Начало--------------------------------------------------------------------------

        //Вызываем метод "GetResults". "GetResults" приниает, адрес указателя на класс или стурктуры "IShellItemArray" и выделяет память под него: https://learn.microsoft.com/ru-ru/windows/win32/api/shobjidl_core/nn-shobjidl_core-ifileopendialog

        IShellItemArray* Shell_Array;

        HRESULT_ = IFileOpenDialog_p->GetResults(&Shell_Array);

        if (HRESULT_ != S_OK)
        {
            //**************************************************
            CoUninitialize();            //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

            IFileOpenDialog_p->Release(); //Освобождаем память.
            //**************************************************


            error = "GetResult: " + get__ErrorText(HRESULT_);

            return  result_flag::winapi_error;
        }

        //----------------------------------------------------------------4-Получим результат выбора Пользовтаеля:Конец--------------------------------------------------------------------------





        //-------------------------------------------------------------5-Получим кол-во выбранных Пользователем элеметов:Начало-------------------------------------------------------------------

        DWORD NumCount;

        HRESULT_ = Shell_Array->GetCount(&NumCount);   //Полуxим кол-во выбранных Пользователем элеметов.  //https://learn.microsoft.com/ru-ru/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishellitemarray-getcount

        if (HRESULT_ != S_OK)
        {
            //**************************************************
            CoUninitialize();            //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

            IFileOpenDialog_p->Release(); //Освобождаем память.

            Shell_Array->Release();     //Освобождаем память.
            //**************************************************

            error = "GetCount: " + get__ErrorText(HRESULT_);

            return  result_flag::winapi_error;
        }

        //NumCount - теперь тут кол-во выбранных Пользователем элеметов.

    //-------------------------------------------------------------5-Получим кол-во выбранных Пользователем элеметов:Конец-------------------------------------------------------------------




    //-------------------------------------------------------------5-Получим текст выбранных Пользователем элеметов:Начало-------------------------------------------------------------------

        vec_SelectFiles.resize(NumCount);


        for (DWORD i = 0; i < NumCount; i++)
        {
            IShellItem* IShellItem_p;

            HRESULT_ = Shell_Array->GetItemAt(i, &IShellItem_p);             //https://learn.microsoft.com/ru-ru/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishellitemarray-getitemat

            if (HRESULT_ != S_OK) { CoUninitialize(); IFileOpenDialog_p->Release(); Shell_Array->Release(); error = "GetItemAt: " + get__ErrorText(HRESULT_); return  result_flag::winapi_error; }


            //---------------------Получаем имя элемента:-------------------
            PWSTR PWSTR_ElementName;

            HRESULT_ = IShellItem_p->GetDisplayName(SIGDN_FILESYSPATH, &PWSTR_ElementName);                  //https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishellitem-getdisplayname  и https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/ne-shobjidl_core-sigdn

            if (HRESULT_ != S_OK) { CoUninitialize(); IFileOpenDialog_p->Release(); Shell_Array->Release(); error = "GetItemAt: " + get__ErrorText(HRESULT_); IShellItem_p->Release(); return  result_flag::winapi_error; }

            //PWSTR_ElementName  - сюда функция выделила память и поместила текст пути выбранной паки. Кодирова UTF-16 wstring.

            add_folder_to_Vector<string_wstring_T2>(i, PWSTR_ElementName, vec_SelectFiles);      //Добавим выбранную папку в вектор результатов.

            CoTaskMemFree(PWSTR_ElementName); //Вызывающая сторона обязана освободить строку, на которую указывает ppszName, когда она больше не нужна. Вызовите CoTaskMemFree, чтобы освободить память.
            //----------------------------------------


            IShellItem_p->Release();   //Освобождаем память, она уже больше не нужна - мы забарли уже все что хотели.
        }

        //-------------------------------------------------------------5-Получим текст выбранных Пользователем элеметов:Конец-------------------------------------------------------------------




        //Собсвенно все, освобождаем, ВСЕ задейстованеые ресурсы.

        //------------------------------------------------------

        CoUninitialize();             //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

        IFileOpenDialog_p->Release();   //Освобождаем память.

        Shell_Array->Release();       //Освобождаем память.

        //------------------------------------------------------


        return result_flag::OK;

    }

    template<typename string_wstring_T1, typename string_wstring_T2> void run_Select_OpenFiles_NewThread(const string_wstring_T1& Preview_Path, const std::vector<COMDLG_FILTERSPEC>& vec_FilterFileTypes, std::function<void(const result_flag result_flag_, const std::string ErrorName, const  std::vector<string_wstring_T2>& vec_SelectFiles)>func_, const flag NumSelect_flag)
    {
        std::thread New_thread(&WinApi__Save_Load_Dialog__class::run_Select_OpenFiles_NewThread_<string_wstring_T1, string_wstring_T2>, this, Preview_Path, vec_FilterFileTypes, func_, NumSelect_flag);

        New_thread.detach();
    }


    template<typename string_wstring_T1, typename string_wstring_T2> const result_flag run_Select_SaveFile_BlockThread(const string_wstring_T1& Preview_Path, string_wstring_T2& selected_file, const std::vector<COMDLG_FILTERSPEC>& vec_FilterFileTypes)
    {


        HRESULT HRESULT_;


        //-----------------------------------------------0-Инициализируем бибилотеку COM:Начало----------------------------------------------

        HRESULT_ = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);       //Инициализируем бибилотеку COM: https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-coinitializeex.  MSDN рекумендует установить флаг "COINIT_DISABLE_OLE1DDE" для того, чтобы какая то там устарвешая модель не использовалась, вот тут: : https://learn.microsoft.com/en-us/windows/win32/learnwin32/initializing-the-com-library

        if (HRESULT_ != S_OK && HRESULT_ != S_FALSE)
        {
            //Значит HRESULT_ == RPC_E_CHANGED_MODE: Предыдущий вызов CoInitializeEx указал несовместимую модель параллелизма для этого потока. Это также может указывать на то, что произошло изменение с нейтрального многопоточного режима на однопоточный.

            error = "CoInitializeEx: " + get__ErrorText(HRESULT_);

            return result_flag::winapi_error;
        }

        //-----------------------------------------------0-Инициализируем бибилотеку COM:Конец----------------------------------------------



        //--------------------------------------------1-Создаем обьект Диалогвоого окна:Начало-----------------------------------------------------

        IFileSaveDialog* IFileSaveDialog_p = NULL;

        HRESULT_ = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_IFileSaveDialog, (LPVOID*)&IFileSaveDialog_p);    //Создаем экземпляр диалогового окна. Первый параметр "CLSID_FileSaveDialog", это указание функции, что в обьект указателя на "диалоговок окна" нужно поместить именно указатель на обьект "диалогвого окно". Второй и треьтий парметры - хер знает, что это такое. В примерах они указаны - ну и хорошо. "IID_IFileDialog" - указывает какие методы будут доступны в "pFileDialog", 

        if (HRESULT_ != S_OK)
        {
            CoUninitialize();  //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

            //-----------------------------------------------------------------
            error = "CoCreateInstance: " + get__ErrorText(HRESULT_);

            return  result_flag::winapi_error;
            //-----------------------------------------------------------------
        }

        //IFileSaveDialog_p - теперь тут указатель на обьект класса "IFileSaveDialog", у которого есть свои два метода(https://learn.microsoft.com/ru-ru/windows/win32/api/shobjidl_core/nn-shobjidl_core-ifilesavedialog) и все методы унаследованные от IFileDialog(https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-ifiledialog)

        //--------------------------------------------1-Создаем обьект Диалогвоого окна:Конец-----------------------------------------------------




         //-------------------------------------------------------2-Устанавливаем необходимые опции для Диалогового окна:Начало-------------------------------------------------------------------------------

        DWORD dwFlags; IFileSaveDialog_p->GetOptions(&dwFlags);         //Получаем опции, который установлены по умолчанию для "IFileSaveDialog", чтобы потом убрать или доабвить к ним другие опции.

        //Список опций, которые принимает "SetOptions" можно посомтреть тут:  https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/ne-shobjidl_core-_fileopendialogoptions

        //В данном случае используем только следующие: 

        //-FOS_ALLOWMULTISELECT:   Позволяет пользователю выбрать несколько элементов в диалоговом окне открытия.Обратите внимание, что при установке этого флага для получения этих элементов необходимо использовать интерфейс IFileSaveDialog.
        //-FOS_NODEREFERENCELINKS: Ярлыки не следует рассматривать как целевые элементы. Это позволяет приложению открывать файл .lnk, а не то, на что указывает ярлык.

        HRESULT_ = IFileSaveDialog_p->SetOptions(dwFlags);

        if (HRESULT_ != S_OK)
        {
            error = "SetOptions: " + get__ErrorText(HRESULT_);

            return  result_flag::winapi_error;
        }

        //-------------------------------------------------------2-Устанавливаем необходимые опции для Диалогового окна:Конец-------------------------------------------------------------------------------




        
        //--------------------------------------------------------------Устанавливаем Пользовательский список фильтров по типу файла:Начало----------------------------------------------------------------------------

        IFileSaveDialog_p->SetFileTypes(vec_FilterFileTypes.size(), vec_FilterFileTypes.data());
        
        
        IFileSaveDialog_p->SetFileTypeIndex(1); //  Устанавливаем фильтр по умолчанию, который будет действовать при создании диалоогового окна. Ставим первый элеммент вектора.

        //--------------------------------------------------------------Устанавливаем Пользовательский список фильтров по типу файла:Конец----------------------------------------------------------------------------





        //--------------------------------------------------------------3-Устанавливаем отображение диалогового окна:Конец----------------------------------------------------------------------------


        //--------------------------------------Если Пользовтаель указал конкретный начальный путь на котором должно октрытся диалоговое окно:-------------------
        const result_flag result_flag_ = PreviewSavePath_handler<string_wstring_T1>(IFileSaveDialog_p, Preview_Path);

        if (result_flag_ != result_flag::OK) { error = "PreviewOpenPath_handler: " + error;  return result_flag_; }
        //-------------------------------------------------------------------------------------------------------------------------------------------------------



        HRESULT_ = IFileSaveDialog_p->Show(NULL);     //Через наследование от IModalWindow, вызываем "show": https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-imodalwindow-show

        if (HRESULT_ != S_OK && HRESULT_ != HRESULT_FROM_WIN32(ERROR_CANCELLED))
        {
            //**************************************************
            CoUninitialize();           //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

            IFileSaveDialog_p->Release(); //Освобождаем память.
            //**************************************************

            error = "Show: " + get__ErrorText(HRESULT_);

            return  result_flag::winapi_error;
        }
        else
        {
            if (HRESULT_ == HRESULT_FROM_WIN32(ERROR_CANCELLED))
            {
                //Значит Пользователь закрыл окно - так ничего и не выбрав.

                //**************************************************
                CoUninitialize();             //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

                IFileSaveDialog_p->Release(); //Освобождаем память.
                //**************************************************

                return result_flag::Cancel;
            }
        }

        //--------------------------------------------------------------3-Устанавливаем отображение диалогового окна:Конец----------------------------------------------------------------------------



        //Если код дошел до сюда, значит Пользователь уже выбрал какую то папку и нажал "Открыть".


        //----------------------------------------------------------------4-Получим результат выбора Пользовтаеля:Начало--------------------------------------------------------------------------
        
        //Вызываем метод "GetResult". "GetResult" приниает, адрес указателя на класс или стурктуры "IShellItem" и выделяет память под него: https://learn.microsoft.com/ru-ru/windows/win32/api/shobjidl_core/nf-shobjidl_core-ifiledialog-getresult и https://learn.microsoft.com/ru-ru/windows/win32/api/shobjidl_core/nn-shobjidl_core-ishellitem

        IShellItem* Shell_Item;

        HRESULT_ = IFileSaveDialog_p->GetResult(&Shell_Item);

        if (HRESULT_ != S_OK)
        {
            //**************************************************
            CoUninitialize();            //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

            IFileSaveDialog_p->Release(); //Освобождаем память.
            //**************************************************


            error = "GetResult: " + get__ErrorText(HRESULT_);

            return  result_flag::winapi_error;
        }
        
        //----------------------------------------------------------------4-Получим результат выбора Пользовтаеля:Конец--------------------------------------------------------------------------




       //-------------------------------------------------------------5-Получим текст выбранных Пользователем элеметов:Начало-------------------------------------------------------------------
        



            //---------------------Получаем имя элемента:-------------------
            PWSTR PWSTR_ElementName;

            HRESULT_ = Shell_Item->GetDisplayName(SIGDN_FILESYSPATH, &PWSTR_ElementName);                  //https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishellitem-getdisplayname  и https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/ne-shobjidl_core-sigdn

            if (HRESULT_ != S_OK) { CoUninitialize(); IFileSaveDialog_p->Release(); Shell_Item->Release(); error = "GetItemAt: " + get__ErrorText(HRESULT_); Shell_Item->Release(); return  result_flag::winapi_error; }

            //PWSTR_ElementName  - сюда функция выделила память и поместила текст пути выбранной паки. Кодирова UTF-16 wstring.

            add_SlelectFile(PWSTR_ElementName, selected_file);      //Добавим выбранную папку в вектор результатов.

            CoTaskMemFree(PWSTR_ElementName); //Вызывающая сторона обязана освободить строку, на которую указывает ppszName, когда она больше не нужна. Вызовите CoTaskMemFree, чтобы освободить память.
            //----------------------------------------


            Shell_Item->Release();   //Освобождаем память, она уже больше не нужна - мы забарли уже все что хотели.
        
        
        //-------------------------------------------------------------5-Получим текст выбранных Пользователем элеметов:Конец-------------------------------------------------------------------




        //Собсвенно все, освобождаем, ВСЕ задейстованеые ресурсы.

        //------------------------------------------------------

        CoUninitialize();             //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

        IFileSaveDialog_p->Release();   //Освобождаем память.

        Shell_Item->Release();       //Освобождаем память.

        //------------------------------------------------------


        return result_flag::OK;

    }

    template<typename string_wstring_T1, typename string_wstring_T2> void run_Select_SaveFile_NewThread(const string_wstring_T1& Preview_Path, const std::vector<COMDLG_FILTERSPEC>& vec_FilterFileTypes, std::function<void(const result_flag result_flag_, const std::string ErrorName, const string_wstring_T2& selected_file)>func_)
    {
        std::thread New_thread(&WinApi__Save_Load_Dialog__class::run_Select_SaveFile_NewThread_<string_wstring_T1, string_wstring_T2>, this, Preview_Path, vec_FilterFileTypes, func_);

        New_thread.detach();
    }




    const std::string get__ErrorName() const
    {
        return error;
    }


private:

    std::string error;



    //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    const std::string get__ErrorText(const HRESULT HRESULT_)
    {
        //HRESULT_ - за место HRESULT может быть код ошибки от GetLastError().

        std::string NameError; NameError.resize(1000);  //Выделим с запасом

        DWORD size = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, HRESULT_, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), &NameError[0], NameError.size(), nullptr);  //Получаем текстовый идентификатор ошибки ТОЛЬКО на Английском языке, чтобы не парится с кодировками и прочей фигней.

        NameError.resize(size);  //Сократи до фактического размера.

        return NameError;
    }


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    template<typename string_wstring_T1> const result_flag PreviewOpenPath_handler(IFileOpenDialog* IFileOpenDialog_p, const string_wstring_T1& Preview_Path)
    {


        if (Preview_Path.size() != 0)
        {

            IShellItem* PreiewFolder_Item_p = 0;
            HRESULT HRESULT_;

            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            if (std::is_same<string_wstring_T1, std::string>::value)
            {
                //Значит Пользователь указал "Preview_Path" в std::string в кодировке UTF-8. Так как функция "SHCreateItemFromParsingName" не понимает ничего кроме UTF-16(wstring) - то конвертируем в UTF-16.

                //------------------Конвертируем UTF-8 в UTF-16:-------------------------
                std::wstring Preview_Path_wstring;

                const result_flag res = from_UTF8_to_UTF16wstring((char*)Preview_Path.c_str(), Preview_Path.size(), Preview_Path_wstring);  //Конвертируем UTF-8 в UTF-16

                if (res != result_flag::OK)
                {
                    error = "from_UTF8_to_UTF16wstring:" + error; return res;
                }
                //---------------------------------------------------------------------


                HRESULT_ = SHCreateItemFromParsingName((wchar_t*)Preview_Path_wstring.c_str(), nullptr, IID_IShellItem, (void**)&PreiewFolder_Item_p);       //https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-shcreateitemfromparsingname

            }
            else if (std::is_same<string_wstring_T1, std::wstring>::value)
            {
                //Значит Пользователь указал "Preview_Path" в std::string в кодировке UTF-16(wstring). Ничего дополнительно делать не нужно.

                HRESULT_ = SHCreateItemFromParsingName((wchar_t*)Preview_Path.c_str(), nullptr, IID_IShellItem, (void**)&PreiewFolder_Item_p);              //https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-shcreateitemfromparsingname
            }


            //-------------------------------------------------------------------
            if (HRESULT_ == S_OK)
            {
                IFileOpenDialog_p->SetFolder(PreiewFolder_Item_p);              // Устанавливаем начальную папку для диалогового окна.

                PreiewFolder_Item_p->Release();  // Освобождаем 

                return result_flag::OK;
            }
            else
            {
                error = "SHCreateItemFromParsingName: " + get__ErrorText(HRESULT_);

                return result_flag::winapi_error;
            }

            //-------------------------------------------------------------------

            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


        }

        return result_flag::OK;

    }
    template<typename string_wstring_T1> const result_flag PreviewSavePath_handler(IFileSaveDialog* IFileSaveDialog_p, const string_wstring_T1& Preview_Path)
    {


        if (Preview_Path.size() != 0)
        {

            IShellItem* PreiewFolder_Item_p = 0;
            HRESULT HRESULT_;

            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            if (std::is_same<string_wstring_T1, std::string>::value)
            {
                //Значит Пользователь указал "Preview_Path" в std::string в кодировке UTF-8. Так как функция "SHCreateItemFromParsingName" не понимает ничего кроме UTF-16(wstring) - то конвертируем в UTF-16.

                //------------------Конвертируем UTF-8 в UTF-16:-------------------------
                std::wstring Preview_Path_wstring;

                const result_flag res = from_UTF8_to_UTF16wstring((char*)Preview_Path.c_str(), Preview_Path.size(), Preview_Path_wstring);  //Конвертируем UTF-8 в UTF-16

                if (res != result_flag::OK)
                {
                    error = "from_UTF8_to_UTF16wstring:" + error; return res;
                }
                //---------------------------------------------------------------------


                HRESULT_ = SHCreateItemFromParsingName((wchar_t*)Preview_Path_wstring.c_str(), nullptr, IID_IShellItem, (void**)&PreiewFolder_Item_p);       //https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-shcreateitemfromparsingname

            }
            else if (std::is_same<string_wstring_T1, std::wstring>::value)
            {
                //Значит Пользователь указал "Preview_Path" в std::string в кодировке UTF-16(wstring). Ничего дополнительно делать не нужно.

                HRESULT_ = SHCreateItemFromParsingName((wchar_t*)Preview_Path.c_str(), nullptr, IID_IShellItem, (void**)&PreiewFolder_Item_p);              //https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-shcreateitemfromparsingname
            }


            //-------------------------------------------------------------------
            if (HRESULT_ == S_OK)
            {
                IFileSaveDialog_p->SetFolder(PreiewFolder_Item_p);              // Устанавливаем начальную папку для диалогового окна.

                PreiewFolder_Item_p->Release();  // Освобождаем 

                return result_flag::OK;
            }
            else
            {
                error = "SHCreateItemFromParsingName: " + get__ErrorText(HRESULT_);

                return result_flag::winapi_error;
            }

            //-------------------------------------------------------------------

            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


        }

        return result_flag::OK;

    }

    template<typename string_wstring_T2> const result_flag add_folder_to_Vector (const DWORD i, const PWSTR& PWSTR_ElementName, std::vector<string_wstring_T2>& vec_SelectFolders)
    {
        const result_flag res = add_folder_to_Vector_<string_wstring_T2>(i, PWSTR_ElementName, vec_SelectFolders, std::is_same<string_wstring_T2, std::string>());
       
        return res;
    }
    template<typename string_wstring_T2> const result_flag add_folder_to_Vector_(const DWORD i, const PWSTR& PWSTR_ElementName, std::vector<string_wstring_T2>& vec_SelectFolders, std::true_type)
    {

        //------------------Конвертируем UTF-16 в UTF-8:-------------------------
        std::string string_;

        const result_flag res = from_UTF16wstring_to_UTF8(PWSTR_ElementName, wcslen(PWSTR_ElementName), string_);  //Конвертируем UTF-8 в UTF-16

        if (res != result_flag::OK)
        {
            error = "from_UTF16_to_UTF8wstring:" + error; 
            
            return res;
        }
        //---------------------------------------------------------------------

        vec_SelectFolders[i] = string_;

        return result_flag::OK;

    }
    template<typename string_wstring_T2> const result_flag add_folder_to_Vector_(const DWORD i, const PWSTR& PWSTR_ElementName, std::vector<string_wstring_T2>& vec_SelectFolders, std::false_type)
    {
        vec_SelectFolders[i] = PWSTR_ElementName;

        return result_flag::OK;

    }

    template<typename string_wstring_T2> const result_flag add_SlelectFile(const PWSTR& PWSTR_ElementName, string_wstring_T2& selected_file)
    {
        const result_flag res = add_SlelectFile_<string_wstring_T2>(PWSTR_ElementName, selected_file, std::is_same<string_wstring_T2, std::string>());

        return res;
    }
    template<typename string_wstring_T2> const result_flag add_SlelectFile_(const PWSTR& PWSTR_ElementName, string_wstring_T2& selected_file, std::true_type)
    {

        //------------------Конвертируем UTF-16 в UTF-8:-------------------------
        std::string string_;

        const result_flag res = from_UTF16wstring_to_UTF8(PWSTR_ElementName, wcslen(PWSTR_ElementName), string_);  //Конвертируем UTF-8 в UTF-16

        if (res != result_flag::OK)
        {
            error = "from_UTF16_to_UTF8wstring:" + error;

            return res;
        }
        //---------------------------------------------------------------------

        selected_file = string_;

        return result_flag::OK;

    }
    template<typename string_wstring_T2> const result_flag add_SlelectFile_(const PWSTR& PWSTR_ElementName, string_wstring_T2& selected_file, std::false_type)
    {
        selected_file = PWSTR_ElementName;

        return result_flag::OK;

    }
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    template<typename string_wstring_T1, typename string_wstring_T2> void run_Select_Folder_NewThread_(const string_wstring_T1& Preview_Path, std::function<void(const result_flag result_flag_, const std::string ErrorName, const std::vector<string_wstring_T2>& vec_SelectFolders)>func_, const flag NumSelect_flag)
    {

        std::vector<string_wstring_T2> vec_SelectFolders;

        HRESULT HRESULT_;


        //-----------------------------------------------0-Инициализируем бибилотеку COM:Начало----------------------------------------------

        HRESULT_ = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);       //Инициализируем бибилотеку COM: https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-coinitializeex.  MSDN рекумендует установить флаг "COINIT_DISABLE_OLE1DDE" для того, чтобы какая то там устарвешая модель не использовалась, вот тут: : https://learn.microsoft.com/en-us/windows/win32/learnwin32/initializing-the-com-library

        if (HRESULT_ != S_OK && HRESULT_ != S_FALSE)
        {
            //Значит HRESULT_ == RPC_E_CHANGED_MODE: Предыдущий вызов CoInitializeEx указал несовместимую модель параллелизма для этого потока. Это также может указывать на то, что произошло изменение с нейтрального многопоточного режима на однопоточный.

            error = "CoInitializeEx: " + get__ErrorText(HRESULT_);

            func_(result_flag::winapi_error, error, vec_SelectFolders);

            return;
        }

        //-----------------------------------------------0-Инициализируем бибилотеку COM:Конец----------------------------------------------



        //--------------------------------------------1-Создаем обьект Диалогвоого окна:Начало-----------------------------------------------------

        IFileOpenDialog* IFileOpenDialog_p = NULL;

        HRESULT_ = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_IFileOpenDialog, (LPVOID*)&IFileOpenDialog_p);    //Создаем экземпляр диалогового окна. Первый параметр "CLSID_FileOpenDialog", это указание функции, что в обьект указателя на "диалоговок окна" нужно поместить именно указатель на обьект "диалогвого окно". Второй и треьтий парметры - хер знает, что это такое. В примерах они указаны - ну и хорошо. "IID_IFileDialog" - указывает какие методы будут доступны в "pFileDialog", 

        if (HRESULT_ != S_OK)
        {
            CoUninitialize();  //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

            //-----------------------------------------------------------------
            error = "CoCreateInstance: " + get__ErrorText(HRESULT_);

            func_(result_flag::winapi_error, error, vec_SelectFolders);
            //-----------------------------------------------------------------

            return;
        }

        //IFileOpenDialog_p - теперь тут указатель на обьект класса "IFileOpenDialog", у которого есть свои два метода(https://learn.microsoft.com/ru-ru/windows/win32/api/shobjidl_core/nn-shobjidl_core-ifileopendialog) и все методы унаследованные от IFileDialog(https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-ifiledialog)

        //--------------------------------------------1-Создаем обьект Диалогвоого окна:Конец-----------------------------------------------------




         //-------------------------------------------------------2-Устанавливаем необходимые опции для Диалогового окна:Начало-------------------------------------------------------------------------------

        DWORD dwFlags; IFileOpenDialog_p->GetOptions(&dwFlags);         //Получаем опции, который установлены по умолчанию для "IFileOpenDialog", чтобы потом убрать или доабвить к ним другие опции.

        //Список опций, которые принимает "SetOptions" можно посомтреть тут:  https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/ne-shobjidl_core-_fileopendialogoptions

        //В данном случае используем только следующие: 

        //-FOS_PICKFOLDERS:        Представьте открытое диалоговое окно, предлагающее выбор папок, а не файлов.
        //-FOS_ALLOWMULTISELECT:   Позволяет пользователю выбрать несколько элементов в диалоговом окне открытия.Обратите внимание, что при установке этого флага для получения этих элементов необходимо использовать интерфейс IFileOpenDialog.
        //-FOS_NODEREFERENCELINKS: Ярлыки не следует рассматривать как целевые элементы. Это позволяет приложению открывать файл .lnk, а не то, на что указывает ярлык.


        //`````````````````````````````````````````````````````
        if (NumSelect_flag == flag::One_Select)
        {
            HRESULT_ = IFileOpenDialog_p->SetOptions(dwFlags | FOS_PICKFOLDERS | FOS_NODEREFERENCELINKS);
        }
        else if (NumSelect_flag == flag::Multi_Select)
        {
            HRESULT_ = IFileOpenDialog_p->SetOptions(dwFlags | FOS_PICKFOLDERS | FOS_ALLOWMULTISELECT | FOS_NODEREFERENCELINKS);
        }
        //`````````````````````````````````````````````````````


        if (HRESULT_ != S_OK)
        {
            error = "SetOptions: " + get__ErrorText(HRESULT_);

            func_(result_flag::winapi_error, error, vec_SelectFolders);

            return;
        }

        //-------------------------------------------------------2-Устанавливаем необходимые опции для Диалогового окна:Конец-------------------------------------------------------------------------------




        //--------------------------------------------------------------3-Устанавливаем отображение диалогового окна:Конец----------------------------------------------------------------------------


        //--------------------------------------Если Пользовтаель указал конкретный начальный путь на котором должно октрытся диалоговое окно:-------------------
        const result_flag result_flag_ = PreviewOpenPath_handler<string_wstring_T1>(IFileOpenDialog_p, Preview_Path);

        if (result_flag_ != result_flag::OK) { error = "PreviewOpenPath_handler: " + error; func_(result_flag_, error, vec_SelectFolders); return; }
        //-------------------------------------------------------------------------------------------------------------------------------------------------------



        HRESULT_ = IFileOpenDialog_p->Show(NULL);     //Через наследование от IModalWindow, вызываем "show": https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-imodalwindow-show

        if (HRESULT_ != S_OK && HRESULT_ != HRESULT_FROM_WIN32(ERROR_CANCELLED))
        {
            //**************************************************
            CoUninitialize();           //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

            IFileOpenDialog_p->Release(); //Освобождаем память.
            //**************************************************

            error = "Show: " + get__ErrorText(HRESULT_);

            func_(result_flag::winapi_error, error, vec_SelectFolders);

            return;
        }
        else
        {
            if (HRESULT_ == HRESULT_FROM_WIN32(ERROR_CANCELLED))
            {
                //Значит Пользователь закрыл окно - так ничего и не выбрав.

                //**************************************************
                CoUninitialize();           //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

                IFileOpenDialog_p->Release(); //Освобождаем память.
                //**************************************************

                func_(result_flag::Cancel, error, vec_SelectFolders);

                return;
            }
        }

        //--------------------------------------------------------------3-Устанавливаем отображение диалогового окна:Конец----------------------------------------------------------------------------



        //Если код дошел до сюда, значит Пользователь уже выбрал какую то папку и нажал "Открыть".


        //----------------------------------------------------------------4-Получим результат выбора Пользовтаеля:Начало--------------------------------------------------------------------------

        //Вызываем метод "GetResults". "GetResults" приниает, адрес указателя на класс или стурктуры "IShellItemArray" и выделяет память под него: https://learn.microsoft.com/ru-ru/windows/win32/api/shobjidl_core/nn-shobjidl_core-ifileopendialog

        IShellItemArray* Shell_Array;

        HRESULT_ = IFileOpenDialog_p->GetResults(&Shell_Array);

        if (HRESULT_ != S_OK)
        {
            //**************************************************
            CoUninitialize();            //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

            IFileOpenDialog_p->Release(); //Освобождаем память.
            //**************************************************


            error = "GetResult: " + get__ErrorText(HRESULT_);

            func_(result_flag::winapi_error, error, vec_SelectFolders);

            return;
        }

        //----------------------------------------------------------------4-Получим результат выбора Пользовтаеля:Конец--------------------------------------------------------------------------





        //-------------------------------------------------------------5-Получим кол-во выбранных Пользователем элеметов:Начало-------------------------------------------------------------------

        DWORD NumCount;

        HRESULT_ = Shell_Array->GetCount(&NumCount);   //Полуxим кол-во выбранных Пользователем элеметов.  //https://learn.microsoft.com/ru-ru/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishellitemarray-getcount

        if (HRESULT_ != S_OK)
        {
            //**************************************************
            CoUninitialize();            //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

            IFileOpenDialog_p->Release(); //Освобождаем память.

            Shell_Array->Release();     //Освобождаем память.
            //**************************************************

            error = "GetCount: " + get__ErrorText(HRESULT_);

            func_(result_flag::winapi_error, error, vec_SelectFolders);

            return;
        }

        //NumCount - теперь тут кол-во выбранных Пользователем элеметов.

    //-------------------------------------------------------------5-Получим кол-во выбранных Пользователем элеметов:Конец-------------------------------------------------------------------




    //-------------------------------------------------------------5-Получим текст выбранных Пользователем элеметов:Начало-------------------------------------------------------------------

        vec_SelectFolders.resize(NumCount);


        for (DWORD i = 0; i < NumCount; i++)
        {
            IShellItem* IShellItem_p;

            HRESULT_ = Shell_Array->GetItemAt(i, &IShellItem_p);             //https://learn.microsoft.com/ru-ru/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishellitemarray-getitemat

            if (HRESULT_ != S_OK) { CoUninitialize(); IFileOpenDialog_p->Release(); Shell_Array->Release(); error = "GetItemAt: " + get__ErrorText(HRESULT_); func_(result_flag::winapi_error, error, vec_SelectFolders); return; }


            //---------------------Получаем имя элемента:-------------------
            PWSTR PWSTR_ElementName;

            HRESULT_ = IShellItem_p->GetDisplayName(SIGDN_FILESYSPATH, &PWSTR_ElementName);                  //https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishellitem-getdisplayname  и https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/ne-shobjidl_core-sigdn

            if (HRESULT_ != S_OK) { CoUninitialize(); IFileOpenDialog_p->Release(); Shell_Array->Release(); error = "GetItemAt: " + get__ErrorText(HRESULT_); IShellItem_p->Release(); func_(result_flag::winapi_error, error, vec_SelectFolders); return; }

            //PWSTR_ElementName  - сюда функция выделила память и поместила текст пути выбранной паки. Кодирова UTF-16 wstring.

            add_folder_to_Vector<string_wstring_T2>(i, PWSTR_ElementName, vec_SelectFolders);      //Добавим выбранную папку в вектор результатов.

            CoTaskMemFree(PWSTR_ElementName); //Вызывающая сторона обязана освободить строку, на которую указывает ppszName, когда она больше не нужна. Вызовите CoTaskMemFree, чтобы освободить память.
            //----------------------------------------


            IShellItem_p->Release();   //Освобождаем память, она уже больше не нужна - мы забарли уже все что хотели.
        }

        //-------------------------------------------------------------5-Получим текст выбранных Пользователем элеметов:Конец-------------------------------------------------------------------





        //Собсвенно все, освобождаем, ВСЕ задейстованеые ресурсы.

        //------------------------------------------------------

        CoUninitialize();             //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

        IFileOpenDialog_p->Release();   //Освобождаем память.

        Shell_Array->Release();       //Освобождаем память.

        //------------------------------------------------------


        func_(result_flag::OK, error, vec_SelectFolders);

    }
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    template<typename string_wstring_T1, typename string_wstring_T2> void run_Select_OpenFiles_NewThread_(const string_wstring_T2& Preview_Path, const std::vector<COMDLG_FILTERSPEC>& vec_FilterFileTypes, std::function<void(const result_flag result_flag_, const std::string ErrorName, const std::vector<string_wstring_T2>& vec_SelectFiles)>func_, const flag NumSelect_flag)
    {


        std::vector<string_wstring_T2> vec_SelectFiles;

        HRESULT HRESULT_;


        //-----------------------------------------------0-Инициализируем бибилотеку COM:Начало----------------------------------------------

        HRESULT_ = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);       //Инициализируем бибилотеку COM: https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-coinitializeex.  MSDN рекумендует установить флаг "COINIT_DISABLE_OLE1DDE" для того, чтобы какая то там устарвешая модель не использовалась, вот тут: : https://learn.microsoft.com/en-us/windows/win32/learnwin32/initializing-the-com-library

        if (HRESULT_ != S_OK && HRESULT_ != S_FALSE)
        {
            //Значит HRESULT_ == RPC_E_CHANGED_MODE: Предыдущий вызов CoInitializeEx указал несовместимую модель параллелизма для этого потока. Это также может указывать на то, что произошло изменение с нейтрального многопоточного режима на однопоточный.

            error = "CoInitializeEx: " + get__ErrorText(HRESULT_);

            func_(result_flag::winapi_error, error, vec_SelectFiles);

            return;
        }

        //-----------------------------------------------0-Инициализируем бибилотеку COM:Конец----------------------------------------------



        //--------------------------------------------1-Создаем обьект Диалогвоого окна:Начало-----------------------------------------------------

        IFileOpenDialog* IFileOpenDialog_p = NULL;

        HRESULT_ = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_IFileOpenDialog, (LPVOID*)&IFileOpenDialog_p);    //Создаем экземпляр диалогового окна. Первый параметр "CLSID_FileOpenDialog", это указание функции, что в обьект указателя на "диалоговок окна" нужно поместить именно указатель на обьект "диалогвого окно". Второй и треьтий парметры - хер знает, что это такое. В примерах они указаны - ну и хорошо. "IID_IFileDialog" - указывает какие методы будут доступны в "pFileDialog", 

        if (HRESULT_ != S_OK)
        {
            CoUninitialize();  //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

            //-----------------------------------------------------------------
            error = "CoCreateInstance: " + get__ErrorText(HRESULT_);

            func_(result_flag::winapi_error, error, vec_SelectFiles);
            //-----------------------------------------------------------------

            return;
        }

        //IFileOpenDialog_p - теперь тут указатель на обьект класса "IFileOpenDialog", у которого есть свои два метода(https://learn.microsoft.com/ru-ru/windows/win32/api/shobjidl_core/nn-shobjidl_core-ifileopendialog) и все методы унаследованные от IFileDialog(https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-ifiledialog)

        //--------------------------------------------1-Создаем обьект Диалогвоого окна:Конец-----------------------------------------------------




         //-------------------------------------------------------2-Устанавливаем необходимые опции для Диалогового окна:Начало-------------------------------------------------------------------------------

        DWORD dwFlags; IFileOpenDialog_p->GetOptions(&dwFlags);         //Получаем опции, который установлены по умолчанию для "IFileOpenDialog", чтобы потом убрать или доабвить к ним другие опции.

        //Список опций, которые принимает "SetOptions" можно посомтреть тут:  https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/ne-shobjidl_core-_fileopendialogoptions

        //В данном случае используем только следующие: 

        //-FOS_ALLOWMULTISELECT:   Позволяет пользователю выбрать несколько элементов в диалоговом окне открытия.Обратите внимание, что при установке этого флага для получения этих элементов необходимо использовать интерфейс IFileOpenDialog.
        //-FOS_NODEREFERENCELINKS: Ярлыки не следует рассматривать как целевые элементы. Это позволяет приложению открывать файл .lnk, а не то, на что указывает ярлык.



        //`````````````````````````````````````````````````````
        if (NumSelect_flag == flag::One_Select)
        {
            HRESULT_ = IFileOpenDialog_p->SetOptions(dwFlags | FOS_NODEREFERENCELINKS);
        }
        else if (NumSelect_flag == flag::Multi_Select)
        {
            HRESULT_ = IFileOpenDialog_p->SetOptions(dwFlags | FOS_ALLOWMULTISELECT | FOS_NODEREFERENCELINKS);
        }
        //`````````````````````````````````````````````````````


        if (HRESULT_ != S_OK)
        {
            error = "SetOptions: " + get__ErrorText(HRESULT_);

            func_(result_flag::winapi_error, error, vec_SelectFiles);

            return;
        }

        //-------------------------------------------------------2-Устанавливаем необходимые опции для Диалогового окна:Конец-------------------------------------------------------------------------------






        //--------------------------------------------------------------Устанавливаем Пользовательский список фильтров по типу файла:Начало----------------------------------------------------------------------------

        IFileOpenDialog_p->SetFileTypes(vec_FilterFileTypes.size(), vec_FilterFileTypes.data());


        IFileOpenDialog_p->SetFileTypeIndex(1); //  Устанавливаем фильтр по умолчанию, который будет действовать при создании диалоогового окна. Ставим первый элеммент вектора.

        //--------------------------------------------------------------Устанавливаем Пользовательский список фильтров по типу файла:Конец----------------------------------------------------------------------------








        //--------------------------------------------------------------3-Устанавливаем отображение диалогового окна:Конец----------------------------------------------------------------------------


        //--------------------------------------Если Пользовтаель указал конкретный начальный путь на котором должно октрытся диалоговое окно:-------------------
        const result_flag result_flag_ = PreviewOpenPath_handler<string_wstring_T1>(IFileOpenDialog_p, Preview_Path);

        if (result_flag_ != result_flag::OK) { error = "PreviewOpenPath_handler: " + error; func_(result_flag_, error, vec_SelectFiles); return; }
        //-------------------------------------------------------------------------------------------------------------------------------------------------------



        HRESULT_ = IFileOpenDialog_p->Show(NULL);     //Через наследование от IModalWindow, вызываем "show": https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-imodalwindow-show

        if (HRESULT_ != S_OK && HRESULT_ != HRESULT_FROM_WIN32(ERROR_CANCELLED))
        {
            //**************************************************
            CoUninitialize();           //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

            IFileOpenDialog_p->Release(); //Освобождаем память.
            //**************************************************

            error = "Show: " + get__ErrorText(HRESULT_);

            func_(result_flag::winapi_error, error, vec_SelectFiles);

            return;
        }
        else
        {
            if (HRESULT_ == HRESULT_FROM_WIN32(ERROR_CANCELLED))
            {
                //Значит Пользователь закрыл окно - так ничего и не выбрав.

                //**************************************************
                CoUninitialize();           //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

                IFileOpenDialog_p->Release(); //Освобождаем память.
                //**************************************************

                func_(result_flag::Cancel, error, vec_SelectFiles);

                return;
            }
        }

        //--------------------------------------------------------------3-Устанавливаем отображение диалогового окна:Конец----------------------------------------------------------------------------



        //Если код дошел до сюда, значит Пользователь уже выбрал какую то папку и нажал "Открыть".


        //----------------------------------------------------------------4-Получим результат выбора Пользовтаеля:Начало--------------------------------------------------------------------------

        //Вызываем метод "GetResults". "GetResults" приниает, адрес указателя на класс или стурктуры "IShellItemArray" и выделяет память под него: https://learn.microsoft.com/ru-ru/windows/win32/api/shobjidl_core/nn-shobjidl_core-ifileopendialog

        IShellItemArray* Shell_Array;

        HRESULT_ = IFileOpenDialog_p->GetResults(&Shell_Array);

        if (HRESULT_ != S_OK)
        {
            //**************************************************
            CoUninitialize();            //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

            IFileOpenDialog_p->Release(); //Освобождаем память.
            //**************************************************


            error = "GetResult: " + get__ErrorText(HRESULT_);

            func_(result_flag::winapi_error, error, vec_SelectFiles);

            return;
        }

        //----------------------------------------------------------------4-Получим результат выбора Пользовтаеля:Конец--------------------------------------------------------------------------





        //-------------------------------------------------------------5-Получим кол-во выбранных Пользователем элеметов:Начало-------------------------------------------------------------------

        DWORD NumCount;

        HRESULT_ = Shell_Array->GetCount(&NumCount);   //Полуxим кол-во выбранных Пользователем элеметов.  //https://learn.microsoft.com/ru-ru/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishellitemarray-getcount

        if (HRESULT_ != S_OK)
        {
            //**************************************************
            CoUninitialize();            //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

            IFileOpenDialog_p->Release(); //Освобождаем память.

            Shell_Array->Release();     //Освобождаем память.
            //**************************************************

            error = "GetCount: " + get__ErrorText(HRESULT_);

            func_(result_flag::winapi_error, error, vec_SelectFiles);

            return;
        }

        //NumCount - теперь тут кол-во выбранных Пользователем элеметов.

    //-------------------------------------------------------------5-Получим кол-во выбранных Пользователем элеметов:Конец-------------------------------------------------------------------




    //-------------------------------------------------------------5-Получим текст выбранных Пользователем элеметов:Начало-------------------------------------------------------------------

        vec_SelectFiles.resize(NumCount);


        for (DWORD i = 0; i < NumCount; i++)
        {
            IShellItem* IShellItem_p;

            HRESULT_ = Shell_Array->GetItemAt(i, &IShellItem_p);             //https://learn.microsoft.com/ru-ru/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishellitemarray-getitemat

            if (HRESULT_ != S_OK) { CoUninitialize(); IFileOpenDialog_p->Release(); Shell_Array->Release(); error = "GetItemAt: " + get__ErrorText(HRESULT_); func_(result_flag::winapi_error, error, vec_SelectFiles); return; }


            //---------------------Получаем имя элемента:-------------------
            PWSTR PWSTR_ElementName;

            HRESULT_ = IShellItem_p->GetDisplayName(SIGDN_FILESYSPATH, &PWSTR_ElementName);                  //https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishellitem-getdisplayname  и https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/ne-shobjidl_core-sigdn

            if (HRESULT_ != S_OK) { CoUninitialize(); IFileOpenDialog_p->Release(); Shell_Array->Release(); error = "GetItemAt: " + get__ErrorText(HRESULT_); IShellItem_p->Release(); func_(result_flag::winapi_error, error, vec_SelectFiles); return; }

            //PWSTR_ElementName  - сюда функция выделила память и поместила текст пути выбранной паки. Кодирова UTF-16 wstring.

            add_folder_to_Vector<string_wstring_T2>(i, PWSTR_ElementName, vec_SelectFiles);      //Добавим выбранную папку в вектор результатов.

            CoTaskMemFree(PWSTR_ElementName); //Вызывающая сторона обязана освободить строку, на которую указывает ppszName, когда она больше не нужна. Вызовите CoTaskMemFree, чтобы освободить память.
            //----------------------------------------


            IShellItem_p->Release();   //Освобождаем память, она уже больше не нужна - мы забарли уже все что хотели.
        }

        //-------------------------------------------------------------5-Получим текст выбранных Пользователем элеметов:Конец-------------------------------------------------------------------





        //Собсвенно все, освобождаем, ВСЕ задейстованеые ресурсы.

        //------------------------------------------------------

        CoUninitialize();             //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

        IFileOpenDialog_p->Release();   //Освобождаем память.

        Shell_Array->Release();       //Освобождаем память.

        //------------------------------------------------------


        func_(result_flag::OK, error, vec_SelectFiles);

    }
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    template<typename string_wstring_T1, typename string_wstring_T2> void run_Select_SaveFile_NewThread_(const string_wstring_T1& Preview_Path, const std::vector<COMDLG_FILTERSPEC>& vec_FilterFileTypes, std::function<void(const result_flag result_flag_, const std::string ErrorName, const string_wstring_T2& selected_file)>func_)
    {

        string_wstring_T2 selected_file;


        HRESULT HRESULT_;


        //-----------------------------------------------0-Инициализируем бибилотеку COM:Начало----------------------------------------------

        HRESULT_ = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);       //Инициализируем бибилотеку COM: https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-coinitializeex.  MSDN рекумендует установить флаг "COINIT_DISABLE_OLE1DDE" для того, чтобы какая то там устарвешая модель не использовалась, вот тут: : https://learn.microsoft.com/en-us/windows/win32/learnwin32/initializing-the-com-library

        if (HRESULT_ != S_OK && HRESULT_ != S_FALSE)
        {
            //Значит HRESULT_ == RPC_E_CHANGED_MODE: Предыдущий вызов CoInitializeEx указал несовместимую модель параллелизма для этого потока. Это также может указывать на то, что произошло изменение с нейтрального многопоточного режима на однопоточный.

            error = "CoInitializeEx: " + get__ErrorText(HRESULT_);

            func_(result_flag::winapi_error, error, selected_file);

            return;
        }

        //-----------------------------------------------0-Инициализируем бибилотеку COM:Конец----------------------------------------------



        //--------------------------------------------1-Создаем обьект Диалогвоого окна:Начало-----------------------------------------------------

        IFileSaveDialog* IFileSaveDialog_p = NULL;

        HRESULT_ = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_IFileSaveDialog, (LPVOID*)&IFileSaveDialog_p);    //Создаем экземпляр диалогового окна. Первый параметр "CLSID_FileSaveDialog", это указание функции, что в обьект указателя на "диалоговок окна" нужно поместить именно указатель на обьект "диалогвого окно". Второй и треьтий парметры - хер знает, что это такое. В примерах они указаны - ну и хорошо. "IID_IFileDialog" - указывает какие методы будут доступны в "pFileDialog", 

        if (HRESULT_ != S_OK)
        {
            CoUninitialize();  //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

            //-----------------------------------------------------------------
            error = "CoCreateInstance: " + get__ErrorText(HRESULT_);

            func_(result_flag::winapi_error, error, selected_file);

            return;
            //-----------------------------------------------------------------
        }

        //IFileSaveDialog_p - теперь тут указатель на обьект класса "IFileSaveDialog", у которого есть свои два метода(https://learn.microsoft.com/ru-ru/windows/win32/api/shobjidl_core/nn-shobjidl_core-ifilesavedialog) и все методы унаследованные от IFileDialog(https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-ifiledialog)

        //--------------------------------------------1-Создаем обьект Диалогвоого окна:Конец-----------------------------------------------------




         //-------------------------------------------------------2-Устанавливаем необходимые опции для Диалогового окна:Начало-------------------------------------------------------------------------------

        DWORD dwFlags; IFileSaveDialog_p->GetOptions(&dwFlags);         //Получаем опции, который установлены по умолчанию для "IFileSaveDialog", чтобы потом убрать или доабвить к ним другие опции.

        //Список опций, которые принимает "SetOptions" можно посомтреть тут:  https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/ne-shobjidl_core-_fileopendialogoptions

        //В данном случае используем только следующие: 

        //-FOS_ALLOWMULTISELECT:   Позволяет пользователю выбрать несколько элементов в диалоговом окне открытия.Обратите внимание, что при установке этого флага для получения этих элементов необходимо использовать интерфейс IFileSaveDialog.
        //-FOS_NODEREFERENCELINKS: Ярлыки не следует рассматривать как целевые элементы. Это позволяет приложению открывать файл .lnk, а не то, на что указывает ярлык.

        HRESULT_ = IFileSaveDialog_p->SetOptions(dwFlags);

        if (HRESULT_ != S_OK)
        {
            error = "SetOptions: " + get__ErrorText(HRESULT_);

            func_(result_flag::winapi_error, error, selected_file);

            return;
        }

        //-------------------------------------------------------2-Устанавливаем необходимые опции для Диалогового окна:Конец-------------------------------------------------------------------------------





        //--------------------------------------------------------------Устанавливаем Пользовательский список фильтров по типу файла:Начало----------------------------------------------------------------------------

        IFileSaveDialog_p->SetFileTypes(vec_FilterFileTypes.size(), vec_FilterFileTypes.data());


        IFileSaveDialog_p->SetFileTypeIndex(1); //  Устанавливаем фильтр по умолчанию, который будет действовать при создании диалоогового окна. Ставим первый элеммент вектора.

        //--------------------------------------------------------------Устанавливаем Пользовательский список фильтров по типу файла:Конец----------------------------------------------------------------------------





        //--------------------------------------------------------------3-Устанавливаем отображение диалогового окна:Конец----------------------------------------------------------------------------


        //--------------------------------------Если Пользовтаель указал конкретный начальный путь на котором должно октрытся диалоговое окно:-------------------
        const result_flag result_flag_ = PreviewSavePath_handler<string_wstring_T1>(IFileSaveDialog_p, Preview_Path);

        if (result_flag_ != result_flag::OK) { error = "PreviewOpenPath_handler: " + error;  func_(result_flag_, error, selected_file); return;}
        //-------------------------------------------------------------------------------------------------------------------------------------------------------



        HRESULT_ = IFileSaveDialog_p->Show(NULL);     //Через наследование от IModalWindow, вызываем "show": https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-imodalwindow-show

        if (HRESULT_ != S_OK && HRESULT_ != HRESULT_FROM_WIN32(ERROR_CANCELLED))
        {
            //**************************************************
            CoUninitialize();           //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

            IFileSaveDialog_p->Release(); //Освобождаем память.
            //**************************************************

            error = "Show: " + get__ErrorText(HRESULT_);

            func_(result_flag::winapi_error, error, selected_file);

            return;
        }
        else
        {
            if (HRESULT_ == HRESULT_FROM_WIN32(ERROR_CANCELLED))
            {
                //Значит Пользователь закрыл окно - так ничего и не выбрав.

                //**************************************************
                CoUninitialize();             //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

                IFileSaveDialog_p->Release(); //Освобождаем память.
                //**************************************************

                func_(result_flag::Cancel, error, selected_file);

                return;
            }
        }

        //--------------------------------------------------------------3-Устанавливаем отображение диалогового окна:Конец----------------------------------------------------------------------------



        //Если код дошел до сюда, значит Пользователь уже выбрал какую то папку и нажал "Открыть".


        //----------------------------------------------------------------4-Получим результат выбора Пользовтаеля:Начало--------------------------------------------------------------------------

        //Вызываем метод "GetResult". "GetResult" приниает, адрес указателя на класс или стурктуры "IShellItem" и выделяет память под него: https://learn.microsoft.com/ru-ru/windows/win32/api/shobjidl_core/nf-shobjidl_core-ifiledialog-getresult и https://learn.microsoft.com/ru-ru/windows/win32/api/shobjidl_core/nn-shobjidl_core-ishellitem

        IShellItem* Shell_Item;

        HRESULT_ = IFileSaveDialog_p->GetResult(&Shell_Item);

        if (HRESULT_ != S_OK)
        {
            //**************************************************
            CoUninitialize();            //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

            IFileSaveDialog_p->Release(); //Освобождаем память.
            //**************************************************


            error = "GetResult: " + get__ErrorText(HRESULT_);

            func_(result_flag::winapi_error, error, selected_file);

            return;
        }

        //----------------------------------------------------------------4-Получим результат выбора Пользовтаеля:Конец--------------------------------------------------------------------------




       //-------------------------------------------------------------5-Получим текст выбранных Пользователем элеметов:Начало-------------------------------------------------------------------

        


        //---------------------Получаем имя элемента:-------------------
        PWSTR PWSTR_ElementName;

        HRESULT_ = Shell_Item->GetDisplayName(SIGDN_FILESYSPATH, &PWSTR_ElementName);                  //https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishellitem-getdisplayname  и https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/ne-shobjidl_core-sigdn

        if (HRESULT_ != S_OK) { CoUninitialize(); IFileSaveDialog_p->Release(); Shell_Item->Release(); error = "GetItemAt: " + get__ErrorText(HRESULT_); Shell_Item->Release(); func_(result_flag::winapi_error, error, selected_file); return; }

        //PWSTR_ElementName  - сюда функция выделила память и поместила текст пути выбранной паки. Кодирова UTF-16 wstring.

        add_SlelectFile(PWSTR_ElementName, selected_file);      //Добавим выбранную папку в вектор результатов.

        CoTaskMemFree(PWSTR_ElementName); //Вызывающая сторона обязана освободить строку, на которую указывает ppszName, когда она больше не нужна. Вызовите CoTaskMemFree, чтобы освободить память.
        //----------------------------------------


        Shell_Item->Release();   //Освобождаем память, она уже больше не нужна - мы забарли уже все что хотели.


        //-------------------------------------------------------------5-Получим текст выбранных Пользователем элеметов:Конец-------------------------------------------------------------------




        //Собсвенно все, освобождаем, ВСЕ задейстованеые ресурсы.

        //------------------------------------------------------

        CoUninitialize();             //Для каждого успешного вызова CoInitializeEx необходимо вызвать CoUninitialize перед завершением работы потока. 

        IFileSaveDialog_p->Release();   //Освобождаем память.

        Shell_Item->Release();       //Освобождаем память.

        //------------------------------------------------------


        func_(result_flag::OK, error, selected_file);

    }
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


    //-------------------------------------------------------------------------------------------------------------------------------------------
    const result_flag from_UTF16wstring_to_UTF8(const wchar_t* wchar_utf16_p, const size_t wchar_utf16_size, std::string& string_UTF8)
    {

        const size_t size_before = string_UTF8.size();


        //---------------------------------------------------------Предварительный расчет необходимой длинны под результативный буффер:--------------------------------------------------------------------

        const size_t size_final_byte = WideCharToMultiByte(CP_UTF8, 0, wchar_utf16_p, wchar_utf16_size, &string_UTF8[0], 0, NULL, NULL);  //5ым параметром я должен указать размер буффера под итоговое преобразование из UTF-16 в указанную кодировку в данном примере UTF-8, НО так как я не знаю какой размер будет занимать преобразованные символы, ТО, если вызвать функцию с 0-вым 6-ым парметром, то функция вернет это кол-во байт для финального буффера без завершающего нуля.

        if (size_final_byte == 0)  //Если возвращенное значение равно 0, значит функция не смогла подсчитать итоговое кол-во и значит это ошибка.
        {
            error = "WideCharToMultiByte: " + get__ErrorText(GetLastError());

            return result_flag::winapi_error;
        }

        //size_final_byte - теперь здесь итоговое кол-во байт, которое требуется для итогового буффера string_UTF8, чтобы туда преобразовать UTF-16 в UTF-8.

        //-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------



        try { string_UTF8.resize(size_before + size_final_byte); } catch (const std::bad_alloc& e) { error = e.what(); return  result_flag::bad_alloc; }



        //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

        const size_t status = WideCharToMultiByte(CP_UTF8, 0, wchar_utf16_p, wchar_utf16_size, &string_UTF8[size_before], size_final_byte, NULL, NULL);  //Вызовем функцию WideCharToMultiByte - еще раз, но уже с size_final_byte:


        if (status == 0) //Если возвращенное значение равно 0, значит функция завершилась с ошибкой
        {
            error = "WideCharToMultiByte: " + get__ErrorText(GetLastError());

            string_UTF8.resize(size_before);

            return result_flag::winapi_error;
        }

        //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


        return result_flag::OK;

    }

    const result_flag from_UTF8_to_UTF16wstring(const char* char_UTF8_p, const size_t UTF8_size, std::wstring& wstring_utf16)
    {

        const size_t size_before = wstring_utf16.size();


        //---------------------------------------------------------Предварительный расчет необходимой длинны под результативный буффер:--------------------------------------------------------------------

        const size_t size_final_byte = MultiByteToWideChar(CP_UTF8, 0, char_UTF8_p, UTF8_size, &wstring_utf16[0], 0);  //5ым параметром я должен указать размер буффера в Символах wchar_t - под итоговое преобразование из UTF-8 в UTF-16, НО так как я не знаю какой размер будет занимать преобразованные символы, ТО, если вызвать функцию с 0-вым 6-ым парметром, то функция вернет кол-во Символов wchar_t требуемого для финального буффера.

        if (size_final_byte == 0)  //Если возвращенное значение равно 0, значит функция не смогла подсчитать итоговое кол-во и значит это ошибка.
        {
            error = "MultiByteToWideChar: " + get__ErrorText(GetLastError());

            return result_flag::winapi_error;
        }

        //size_final_byte - теперь здесь итоговое кол-во символов wchar_t, которое требуется для итогового буффера wstring_utf16, чтобы туда преобразовать UTF-8 в UTF-16.

        //-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------



        try { wstring_utf16.resize(size_before + size_final_byte); } catch (const std::bad_alloc& e) { error = e.what(); return  result_flag::bad_alloc; }



        //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

        const size_t status = MultiByteToWideChar(CP_UTF8, 0, char_UTF8_p, UTF8_size, &wstring_utf16[size_before], size_final_byte);  //Вызовем функцию WideCharToMultiByte - еще раз, но уже с size_final_byte:


        if (status == 0) //Если возвращенное значение равно 0, значит функция завершилась с ошибкой
        {
            error = "MultiByteToWideChar: " + get__ErrorText(GetLastError());

            return result_flag::winapi_error;
        }

        //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


        return result_flag::OK;

    }
    //-------------------------------------------------------------------------------------------------------------------------------------------

};
