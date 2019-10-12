#pragma once
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include "EnterSeed.h"
#include "MouseListener.h"

namespace generator {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Runtime::InteropServices;

	unsigned char * generate(unsigned char * input, int input_length, int space, int output_length) {
		unsigned char * output = (unsigned char*)malloc((output_length + 1) * sizeof(unsigned char));
		output[output_length] = '\0';
		unsigned char next = 0;
		for (int i = 0; i < output_length; i += input_length) {
			int length = (i + input_length < output_length) ? input_length : (output_length - i);
			for (int j = 0; j < length; j++) {
				for (int k = 0; k < 8; k++) {
					if (((i + j) * 8 + k) % space) {
						unsigned char temp = (input[j] >> (7 - k)) & 1;
						if (((next >> 1) & 1) == 0) {
							input[j] &= ~(unsigned char)(1 << (7 - k));
						}
						else {
							input[j] |= (unsigned char)(1 << (7 - k));
						}
						next ^= temp;
					}
					else {
						unsigned char rnd = (rand() % 2);
						if (rnd == 0) {
							input[j] &= ~(unsigned char)(1 << (7 - k));
						}
						else {
							input[j] |= (unsigned char)(1 << (7 - k));
						}
						next ^= rnd;
					}
					next <<= 1;
				}
				output[i + j] = next;
			}
		}
		return output;
	};

	unsigned char * generateStandartInput(int input_length) {
		unsigned char * input = (unsigned char*)malloc((input_length + 1) * sizeof(unsigned char));
		unsigned char next = 0;
		for (int i = 0; i < input_length; i++) {
			for (int j = 0; j < 8; j++) {
				unsigned char rnd = (rand() % 2);
				next ^= rnd;
				next <<= 1;
			}
			input[i] = next;
		}
		return input;
	}

	unsigned char * generateInputFromSeed(int seed, int input_length) {
		return generate((unsigned char*)&seed, 4, INT_MAX, input_length);
	}

	/// <summary>
	/// —водка дл€ MyForm
	/// </summary>
	public ref class MyForm : public System::Windows::Forms::Form
	{
	public:
		MyForm(void)
		{
			InitializeComponent();
			//
			//TODO: добавьте код конструктора
			//
		}

	protected:
		/// <summary>
		/// ќсвободить все используемые ресурсы.
		/// </summary>
		~MyForm()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Button^  standartButton;
	private: System::Windows::Forms::Button^  mouseButton;
	private: System::Windows::Forms::Button^  seedButton;
	protected:



	private: System::Windows::Forms::NumericUpDown^  inputLengthNumeric;

	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::Label^  label2;
	private: System::Windows::Forms::NumericUpDown^  spaceNumeric;

	private: System::Windows::Forms::Label^  label3;
	private: System::Windows::Forms::TextBox^  fileTextBox;

	private: System::Windows::Forms::Label^  label4;
	private: System::Windows::Forms::NumericUpDown^  outputLengthNumeric;

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
			this->standartButton = (gcnew System::Windows::Forms::Button());
			this->mouseButton = (gcnew System::Windows::Forms::Button());
			this->seedButton = (gcnew System::Windows::Forms::Button());
			this->inputLengthNumeric = (gcnew System::Windows::Forms::NumericUpDown());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->spaceNumeric = (gcnew System::Windows::Forms::NumericUpDown());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->fileTextBox = (gcnew System::Windows::Forms::TextBox());
			this->label4 = (gcnew System::Windows::Forms::Label());
			this->outputLengthNumeric = (gcnew System::Windows::Forms::NumericUpDown());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->inputLengthNumeric))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->spaceNumeric))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->outputLengthNumeric))->BeginInit();
			this->SuspendLayout();
			// 
			// standartButton
			// 
			this->standartButton->Location = System::Drawing::Point(100, 123);
			this->standartButton->Name = L"standartButton";
			this->standartButton->Size = System::Drawing::Size(75, 23);
			this->standartButton->TabIndex = 0;
			this->standartButton->Text = L"Standart";
			this->standartButton->UseVisualStyleBackColor = true;
			this->standartButton->Click += gcnew System::EventHandler(this, &MyForm::standartButton_Click);
			// 
			// mouseButton
			// 
			this->mouseButton->Location = System::Drawing::Point(100, 166);
			this->mouseButton->Name = L"mouseButton";
			this->mouseButton->Size = System::Drawing::Size(75, 23);
			this->mouseButton->TabIndex = 1;
			this->mouseButton->Text = L"Mouse";
			this->mouseButton->UseVisualStyleBackColor = true;
			this->mouseButton->Click += gcnew System::EventHandler(this, &MyForm::mouseButton_Click);
			// 
			// seedButton
			// 
			this->seedButton->Location = System::Drawing::Point(100, 213);
			this->seedButton->Name = L"seedButton";
			this->seedButton->Size = System::Drawing::Size(75, 23);
			this->seedButton->TabIndex = 2;
			this->seedButton->Text = L"Seed";
			this->seedButton->UseVisualStyleBackColor = true;
			this->seedButton->Click += gcnew System::EventHandler(this, &MyForm::seedButton_Click);
			// 
			// inputLengthNumeric
			// 
			this->inputLengthNumeric->Location = System::Drawing::Point(133, 32);
			this->inputLengthNumeric->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 2147483647, 0, 0, 0 });
			this->inputLengthNumeric->Name = L"inputLengthNumeric";
			this->inputLengthNumeric->Size = System::Drawing::Size(120, 20);
			this->inputLengthNumeric->TabIndex = 3;
			this->inputLengthNumeric->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1000, 0, 0, 0 });
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(32, 39);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(67, 13);
			this->label1->TabIndex = 4;
			this->label1->Text = L"Input Length";
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->Location = System::Drawing::Point(32, 66);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(38, 13);
			this->label2->TabIndex = 5;
			this->label2->Text = L"Space";
			// 
			// spaceNumeric
			// 
			this->spaceNumeric->Location = System::Drawing::Point(133, 59);
			this->spaceNumeric->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 2147483647, 0, 0, 0 });
			this->spaceNumeric->Name = L"spaceNumeric";
			this->spaceNumeric->Size = System::Drawing::Size(120, 20);
			this->spaceNumeric->TabIndex = 6;
			this->spaceNumeric->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1000, 0, 0, 0 });
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->Location = System::Drawing::Point(32, 9);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(51, 13);
			this->label3->TabIndex = 7;
			this->label3->Text = L"FileName";
			// 
			// fileTextBox
			// 
			this->fileTextBox->Location = System::Drawing::Point(133, 9);
			this->fileTextBox->Name = L"fileTextBox";
			this->fileTextBox->Size = System::Drawing::Size(100, 20);
			this->fileTextBox->TabIndex = 8;
			this->fileTextBox->Text = L"generated.txt";
			// 
			// label4
			// 
			this->label4->AutoSize = true;
			this->label4->Location = System::Drawing::Point(32, 93);
			this->label4->Name = L"label4";
			this->label4->Size = System::Drawing::Size(75, 13);
			this->label4->TabIndex = 9;
			this->label4->Text = L"Output Length";
			// 
			// outputLengthNumeric
			// 
			this->outputLengthNumeric->Location = System::Drawing::Point(133, 86);
			this->outputLengthNumeric->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 2147483647, 0, 0, 0 });
			this->outputLengthNumeric->Name = L"outputLengthNumeric";
			this->outputLengthNumeric->Size = System::Drawing::Size(120, 20);
			this->outputLengthNumeric->TabIndex = 10;
			this->outputLengthNumeric->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1000000, 0, 0, 0 });
			// 
			// MyForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(284, 261);
			this->Controls->Add(this->outputLengthNumeric);
			this->Controls->Add(this->label4);
			this->Controls->Add(this->fileTextBox);
			this->Controls->Add(this->label3);
			this->Controls->Add(this->spaceNumeric);
			this->Controls->Add(this->label2);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->inputLengthNumeric);
			this->Controls->Add(this->seedButton);
			this->Controls->Add(this->mouseButton);
			this->Controls->Add(this->standartButton);
			this->Name = L"MyForm";
			this->Text = L"MyForm";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->inputLengthNumeric))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->spaceNumeric))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->outputLengthNumeric))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
		private: void saveFile(unsigned char * output) {
			FILE * file;
			fopen_s(&file, (char*)(void*)Marshal::StringToHGlobalAnsi(this->fileTextBox->Text), "wb");
			fwrite(output, 1, System::Decimal::ToInt32(this->outputLengthNumeric->Value), file);
			fclose(file);
			free(output);
		}
#pragma endregion
		private: System::Void standartButton_Click(System::Object^  sender, System::EventArgs^  e) {
			unsigned char * input = generateStandartInput(System::Decimal::ToInt32(this->inputLengthNumeric->Value));
			saveFile(generate(input, System::Decimal::ToInt32(this->inputLengthNumeric->Value), System::Decimal::ToInt32(this->spaceNumeric->Value), System::Decimal::ToInt32(this->outputLengthNumeric->Value)));
			free(input);
		}
		private: System::Void mouseButton_Click(System::Object^  sender, System::EventArgs^  e) {
			MouseListener ^ mouseListener = gcnew MouseListener(System::Decimal::ToInt32(this->inputLengthNumeric->Value));
			mouseListener->ShowDialog();
			saveFile(generate(mouseListener->getInputData(), System::Decimal::ToInt32(this->inputLengthNumeric->Value), System::Decimal::ToInt32(this->spaceNumeric->Value), System::Decimal::ToInt32(this->outputLengthNumeric->Value)));
		}
		private: System::Void seedButton_Click(System::Object^  sender, System::EventArgs^  e) {
			MyForm1 ^ enterSeed = gcnew MyForm1();
			enterSeed->ShowDialog();
			saveFile(generate(generateInputFromSeed(enterSeed->getSeed(), System::Decimal::ToInt32(this->inputLengthNumeric->Value)), System::Decimal::ToInt32(this->inputLengthNumeric->Value), System::Decimal::ToInt32(this->spaceNumeric->Value), System::Decimal::ToInt32(this->outputLengthNumeric->Value)));
		}
	};
}
