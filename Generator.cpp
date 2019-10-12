#include "Generator.h"
using namespace generator;
using namespace System;
using namespace System::Windows::Forms;

[STAThread]
void main(array<String^>^ arg) {
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);

	MyForm form; //WinFormsTest - имя вашего проекта
	Application::Run(%form);
}