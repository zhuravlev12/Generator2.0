#pragma once
#include <stdlib.h>

namespace generator {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// —водка дл€ MouseListener
	/// </summary>
	public ref class MouseListener : public System::Windows::Forms::Form
	{
	public:
		MouseListener(int input_length)
		{
			InitializeComponent();
			//
			//TODO: добавьте код конструктора
			//
			this->input_length = input_length;
			this->inputted = 0;
			this->input = (unsigned char*)malloc(input_length);
		}

	protected:
		/// <summary>
		/// ќсвободить все используемые ресурсы.
		/// </summary>
		~MouseListener()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Label^  label1;
	protected:

	private:
		/// <summary>
		/// ќб€зательна€ переменна€ конструктора.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// “ребуемый метод дл€ поддержки конструктора Ч не измен€йте 
		/// содержимое этого метода с помощью редактора кода.
		/// </summary>
		void InitializeComponent(void)
		{
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->SuspendLayout();
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(52, 125);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(179, 13);
			this->label1->TabIndex = 0;
			this->label1->Text = L"Move your mouse inside this window";
			// 
			// MouseListener
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(284, 261);
			this->Controls->Add(this->label1);
			this->Name = L"MouseListener";
			this->Text = L"MouseListener";
			this->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &MouseListener::mouseMove);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private:
		int input_length;
		int inputted;
		unsigned char * input;
		void mouseMove(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
			input[inputted] = e->X;
			inputted++;
			if (inputted > input_length) {
				this->Hide();
			}
			input[inputted] = e->Y;
			inputted++;
			if (inputted > input_length) {
				this->Hide();
			}
		}
	public:
		unsigned char * getInputData() {
			return this->input;
		}
	};
}
