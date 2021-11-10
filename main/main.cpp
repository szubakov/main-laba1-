#include <Windows.h>
#include <iostream>
//Точка входа для Windows приложение
HINSTANCE g_hInstance;

int g_nCmdShow;
HWND g_mainWnd;
// RGB заливки
int g_FillColor = -1;
int g_RowStart = -1;
int g_ColStart = -1;
int g_Width = -1;
int g_Height = -1;
HBITMAP g_hBitmap = NULL;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL InitAppClass();
BOOL InitWindow();
WPARAM StartMessageLoop();

bool loadImage()
{
	// загружаем картинку из файла
	g_hBitmap = (HBITMAP)LoadImage(NULL, L".\\laba1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

	if (!g_hBitmap)
		return false;

	// получаем параметры картинки
	BITMAP bm;
	GetObject(g_hBitmap, sizeof(BITMAP), &bm);

	// получаем данные 
	uint8_t* data = (uint8_t*)bm.bmBits;
	// строки(ряды) и столбцы, в которых лежит искомая фигура
	int min_figure_row = -1;
	int max_figure_row = -1;
	int min_figure_col = -1;
	int max_figure_col = -1;
	
	for (int row = 0; row < bm.bmHeight; ++row)
	{
		
		int col_min = -1;
		int col_max = -1;

		for (int col = 0; col < bm.bmWidth; ++col)
		{
			// цикл для обхода всех пикселей

			// берем данные RGB каналов
			uint8_t& r = data[row * bm.bmWidth * 3 + col * 3];
			uint8_t& g = data[row * bm.bmWidth * 3 + col * 3 + 1];
			uint8_t& b = data[row * bm.bmWidth * 3 + col * 3 + 2];
			
			int rgb = ((int)b << 16) | ((int)g << 8) | (int)r;

			if (g_FillColor == -1)
			{
				// цвет заливки не был инициализирован
				// за цвет заливки принимаем первый пиксель
				g_FillColor = rgb;
			}

			// текущий пиксель является фоновой заливкой
			const bool is_fill = rgb == g_FillColor;

			if (!is_fill)
			{
				

				// если в ряду еще не был найден - инициализируем
				if (col_min < 0) col_min = col;
			}
			else
			{
				

				// если были найдены пиксели фигуры в ряду - заканчиваем предыдущим столбцом
				if (col_min > 0 && col_max < 0) col_max = col - 1;

				r = g = b = 255;
			}
		}

		if (col_min > 0)
		{
			
			if (min_figure_row < 0) min_figure_row = row;

			
			const int col_d = col_max - col_min;
			if (col_d > max_figure_col - min_figure_col)
			{
				min_figure_col = col_min;
				max_figure_col = col_max;
			}
		}
		else
		{
			
			if (min_figure_row > 0 && max_figure_row < 0) max_figure_row = row - 1;
		}
	}

	g_Width = max_figure_col - min_figure_col;
	g_Height = max_figure_row - min_figure_row;
	g_RowStart = min_figure_row;
	g_ColStart = min_figure_col;

	return true;
}


//Точка входа в WinПрограмы
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//Установим и инициализируем
	g_hInstance = hInstance;
	g_nCmdShow = nCmdShow;

	if (!loadImage())
		return 0;
	if (!InitAppClass())
		return 0;
	if (!InitWindow())
		return 0;

	return StartMessageLoop();
}
//Регистрация класс окно приложение
BOOL InitAppClass()
{
	ATOM class_id;//Идентификатор
	WNDCLASS wc;//Обект  класса окно приложени
	memset(&wc, 0, sizeof(wc));
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = g_hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = L"Practika1";
	//Регистрация класс
	class_id = RegisterClass(&wc);
	if (class_id != 0)
		return TRUE;
	return FALSE;

}
BOOL InitWindow()
{
	//Теперь создаем окна из 500 500 в центре десктопа
	g_mainWnd = CreateWindow(L"Practika1", L"1", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		500, 500, 0, 0, g_hInstance, 0);
	//Если окно не создано
	if (!g_mainWnd)
		return FALSE;
	ShowWindow(g_mainWnd, g_nCmdShow);//Выхов окна
	UpdateWindow(g_mainWnd);//Обновление 
	return TRUE;
}
//Обработка сообщение
WPARAM StartMessageLoop()
{
	MSG msg;
	while (1)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;
			DispatchMessage(&msg);
		}
	}
	return msg.wParam;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		DeleteObject(g_hBitmap);
		return 0;
	}
	//WM_PAINT перерисока окна
	case WM_PAINT:
	{
		PAINTSTRUCT     ps;
		HDC             hdc;
		BITMAP          bitmap;
		HDC             hdcMem;
		HGDIOBJ         oldBitmap;
		RECT r;

		GetClientRect(hWnd, &r);

		// вычисляем левый верхний угол выводимой фигуры, чтобы она была поцентру снизу
		// ограничиваемся нулем при вычислениях
		int desired_x = r.right / 2 - g_Width / 2;
		int desired_y = r.bottom - g_Height;

		if (desired_x < 0) desired_x = 0;
		if (desired_y < 0) desired_y = 0;

		hdc = BeginPaint(hWnd, &ps);

		hdcMem = CreateCompatibleDC(hdc);
		oldBitmap = SelectObject(hdcMem, g_hBitmap);

		GetObject(g_hBitmap, sizeof(bitmap), &bitmap);

		if (r.right > g_Width && r.bottom > g_Height)
		{
			
			BitBlt(hdc, desired_x, desired_y, r.right, r.bottom, hdcMem, g_ColStart, bitmap.bmHeight - (g_RowStart + g_Height), SRCCOPY);
		}
		else
		{
			
			const int l = (r.right < r.bottom) ? r.right : r.bottom;
			int x_start, y_start;
			if (l == r.right)
			{
			
				x_start = desired_x;
				y_start = r.bottom - l;
			}
			else
			{
			
				x_start = r.right / 2 - l / 2;
				y_start = desired_y;
			}

			StretchBlt(hdc, x_start, y_start, l, l, hdcMem, g_ColStart, bitmap.bmHeight - (g_RowStart + g_Height), g_Width, g_Height, SRCCOPY);
		}

		SelectObject(hdcMem, oldBitmap);
		DeleteDC(hdcMem);

		EndPaint(hWnd, &ps);
		break;
	}
	default:
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
