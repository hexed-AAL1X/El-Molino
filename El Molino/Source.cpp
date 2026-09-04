//  Librerias
#include <iostream>
#include <conio.h>
#include <Windows.h>
#include <stdlib.h>
#include <ctime>
//  Fin Librerias

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);  //  C�digo para usar colores

// ===== Framebuffer: pinta en memoria y vuelca la pantalla de una vez =====
enum { SCR_W = 210, SCR_H = 49 };
static CHAR_INFO gScr[SCR_H][SCR_W];
static WORD gScrAttr = 7;
static int gScrX = 1, gScrY = 1;
static bool gScrOn = false;

static bool scrIsMingwTerminal() {
	return getenv("MSYSTEM") != NULL || getenv("MINGW_PREFIX") != NULL;
}

// Atributo Win32 (0-15) -> secuencia ANSI (mintty/Git Bash)
static void scrApplyColor(WORD attr) {
	if (scrIsMingwTerminal()) {
		// Win: BGR bits 0=black,1=blue,2=green,3=cyan,4=red,5=mag,6=yel,7=white
		static const int winToAnsi[8] = { 30, 34, 32, 36, 31, 35, 33, 37 };
		int code = winToAnsi[attr & 7];
		if (attr & 8) code += 60; // brillante 90-97
		std::cout << "\033[" << code << "m";
	}
	else {
		SetConsoleTextAttribute(hConsole, attr);
	}
}

static void scrResizeConsole() {
	COORD largest = GetLargestConsoleWindowSize(hConsole);
	SHORT w = (SHORT)SCR_W;
	SHORT h = (SHORT)SCR_H;
	if (largest.X > 0 && largest.X < w) w = largest.X;
	if (largest.Y > 0 && largest.Y < h) h = largest.Y;

	SMALL_RECT tiny = { 0, 0, 1, 1 };
	SetConsoleWindowInfo(hConsole, TRUE, &tiny);
	COORD buf = { w, h };
	SetConsoleScreenBufferSize(hConsole, buf);
	SMALL_RECT win = { 0, 0, (SHORT)(w - 1), (SHORT)(h - 1) };
	SetConsoleWindowInfo(hConsole, TRUE, &win);
}

static void color_impl(HANDLE h, int c) {
	gScrAttr = (WORD)c;
	if (!gScrOn) scrApplyColor((WORD)c);
}
#define color color_impl

struct GameOut {
	GameOut& operator<<(char ch) {
		if (gScrOn) {
			if (gScrX >= 1 && gScrX <= SCR_W && gScrY >= 1 && gScrY <= SCR_H) {
				CHAR_INFO& cell = gScr[gScrY - 1][gScrX - 1];
				cell.Char.AsciiChar = ch; // byte CP437 original (219, 220, 223, ...)
				cell.Attributes = gScrAttr;
			}
		}
		else {
			std::cout.put(ch);
		}
		return *this;
	}
	GameOut& operator<<(const char* s) {
		if (!s) return *this;
		if (gScrOn) {
			return (*this) << (char)(s[0] ? (unsigned char)s[0] : ' ');
		}
		while (*s) std::cout.put(*s++);
		return *this;
	}
	GameOut& operator<<(std::ostream& (*manip)(std::ostream&)) {
		if (!gScrOn) manip(std::cout);
		return *this;
	}
};
static GameOut gameOut;

static void scrBeginFrame() {
	gScrOn = true;
	gScrAttr = 7;
	for (int y = 0; y < SCR_H; ++y) {
		for (int x = 0; x < SCR_W; ++x) {
			gScr[y][x].Char.AsciiChar = ' ';
			gScr[y][x].Attributes = 7;
		}
	}
}

static void scrFlushToCout() {
	WORD prev = 0xFFFF;
	if (!scrIsMingwTerminal()) {
		COORD home = { 0, 0 };
		SetConsoleCursorPosition(hConsole, home);
	}
	else {
		std::cout << "\033[H"; // cursor al inicio (ANSI)
	}
	for (int y = 0; y < SCR_H; ++y) {
		for (int x = 0; x < SCR_W; ++x) {
			WORD a = gScr[y][x].Attributes;
			if (a != prev) {
				scrApplyColor(a);
				prev = a;
			}
			std::cout.put(gScr[y][x].Char.AsciiChar);
		}
		std::cout.put('\n');
	}
	if (scrIsMingwTerminal()) std::cout << "\033[0m";
	std::cout.flush();
}

static void scrEndFrame() {
	gScrOn = false;

	if (!scrIsMingwTerminal()) {
		scrResizeConsole();
		COORD bufSize = { (SHORT)SCR_W, (SHORT)SCR_H };
		COORD bufCoord = { 0, 0 };
		SMALL_RECT region = { 0, 0, (SHORT)(SCR_W - 1), (SHORT)(SCR_H - 1) };
		if (WriteConsoleOutputA(hConsole, &gScr[0][0], bufSize, bufCoord, &region)) {
			return;
		}
	}

	// MinGW o si WriteConsoleOutput falla: volcado completo (ANSI en mintty)
	if (scrIsMingwTerminal()) std::cout << "\033[2J\033[H";
	else system("cls");
	scrFlushToCout();
}

static void scrHideCursor() {
	CONSOLE_CURSOR_INFO ci;
	ci.dwSize = 1;
	ci.bVisible = FALSE;
	SetConsoleCursorInfo(hConsole, &ci);
	if (scrIsMingwTerminal()) std::cout << "\033[?25l";
}

//  Define (color ya definido arriba como color_impl)

//9.20
//  Funciones
void Molino_v9_20();  //  Funci�n El_Molino Versi�n 9.20 beta
void Men�(int* x, int* y, int* YFlch, int* XFlch, short* Color, int* ColorL, short* Contr, short& cont, char* Nombre, short& contL, short* Turno, short* ColorD, short& R, short* ColorF, short* ColorFJ, bool Molino, short* ColorA);  //  Funci�n Men�, Env�o: P�rametro por referencia puntero
void Dibuja_Torre_Izquierda(int& X, int& Y, short* Color);  //  Funci�n Dibuja_Torre_Izquierda, Env�o: P�rametro por referencia y referencia puntero
void Dibuja_Torre_Derecha(int& X, int& y);  //  Funci�n Dibuja_Torre_Derecha, Env�o: P�rametro por referencia
void Dibuja_L_Titulo(int& X, int& Y);
void Dibuja_E_Titulo(int& X, int& Y);
void Dibuja_I_Titulo(int& X, int& Y);
void Dibuja_Cubito(int& X, int& Y);  //  Funci�n Dibuja_Cubito, Env�o: P�rametro por referencia
void Dibuja_Ventilador(int& X, int& Y);  //  Funci�n Dibuja_Ventilador, Env�o: P�rametro por referencia
void C_2022_Programaci�n1(int& X, int& Y);  //  Funci�n C_2022_Programaci�n1, Env�o: P�rametro por referencia
void Dibuja_OJugar(int& X, int& Y);  //  Funci�n Dibuja_OJugar, Env�o: P�rametro por referencia
void Dibuja_OOpciones(int& X, int& Y);  //  Funci�n Dibuja_OOpciones, Env�o: P�rametro por referencia
void Dibuja_OInstrucciones(int& X, int& Y);  //  Funci�n Dibuja_OInstrucciones, Env�o: P�rametro por referencia
void Dibuja_OSalir(int& X, int& Y);  //  Funci�n Dibuja_OSalir, Env�o: P�rametro por referencia
void Dibuja_Flechita(int& X, int& Y, int* YFlch, int* XFlch, int* ColorL);  //  Funci�n Dibuja_Flechita, Env�o: P�rametro por referencia y referencia puntero
void Jugar(int& X, int& Y, int* ColorL, short& cont, char* Nombre, short& contL);  //  Funci�n Jugar, Env�o: P�rametro por referencia
void LetrasJ_A(int& X, int& Y, short& cont, char* Nombre);
void LetrasJ_B(int& X, int& Y, short& cont, char* Nombre);
void LetrasJ_C(int& X, int& Y, short& cont, char* Nombre);
void LetrasJ_D(int& X, int& Y, short& cont, char* Nombre);
void LetrasJ_E(int& X, int& Y, short& cont, char* Nombre);
void LetrasJ_F(int& X, int& Y, short& cont, char* Nombre);
void LetrasJ_G(int& X, int& Y, short& cont, char* Nombre);
void LetrasJ_H(int& X, int& Y, short& cont, char* Nombre);
void LetrasJ_I(int& X, int& Y, short& cont, char* Nombre);
void LetrasJ_J(int& X, int& Y, short& cont, char* Nombre);
void LetrasJ_K(int& X, int& Y, short& cont, char* Nombre);
void LetrasJ_L(int& X, int& Y, short& cont, char* Nombre);
void LetrasJ_M(int& X, int& Y, short& cont, char* Nombre);
void LetrasJ_N(int& X, int& Y, short& cont, char* Nombre);
void LetrasJ_O(int& X, int& Y, short& cont, char* Nombre);
void LetrasJ_P(int& X, int& Y, short& cont, char* Nombre);
void LetrasJ_Q(int& X, int& Y, short& cont, char* Nombre);
void LetrasJ_R(int& X, int& Y, short& cont, char* Nombre);
void LetrasJ_S(int& X, int& Y, short& cont, char* Nombre);
void LetrasJ_T(int& X, int& Y, short& cont, char* Nombre);
void LetrasJ_U(int& X, int& Y, short& cont, char* Nombre);
void LetrasJ_V(int& X, int& Y, short& cont, char* Nombre);
void LetrasJ_W(int& X, int& Y, short& cont, char* Nombre);
void LetrasJ_X(int& X, int& Y, short& cont, char* Nombre);
void LetrasJ_Y(int& X, int& Y, short& cont, char* Nombre);
void LetrasJ_Z(int& X, int& Y, short& cont, char* Nombre);
void Letras3J_A_D(int& X, int& Y, short& cont, char* Nombre);
void Letras3J_E_H(int& X, int& Y, short& cont, char* Nombre);
void Letras3J_I_L(int& X, int& Y, short& cont, char* Nombre);
void Letras3J_M_P(int& X, int& Y, short& cont, char* Nombre);
void Letras3J_Q_T(int& X, int& Y, short& cont, char* Nombre);
void Letras3J_U_X(int& X, int& Y, short& cont, char* Nombre);
void Letras3J_Y_Z(int& X, int& Y, short& cont, char* Nombre);
void Letras3BJ_A_D(int& X, int& Y, short& cont, char* Nombre);
void Letras3BJ_E_H(int& X, int& Y, short& cont, char* Nombre);
void Letras3BJ_I_L(int& X, int& Y, short& cont, char* Nombre);
void Letras3BJ_M_P(int& X, int& Y, short& cont, char* Nombre);
void Letras3BJ_Q_T(int& X, int& Y, short& cont, char* Nombre);
void Letras3BJ_U_X(int& X, int& Y, short& cont, char* Nombre);
void Letras3BJ_Y_Z(int& X, int& Y, short& cont, char* Nombre);
void Letras3CJ_A_D(int& X, int& Y, short& cont, char* Nombre);
void Letras3CJ_E_H(int& X, int& Y, short& cont, char* Nombre);
void Letras3CJ_I_L(int& X, int& Y, short& cont, char* Nombre);
void Letras3CJ_M_P(int& X, int& Y, short& cont, char* Nombre);
void Letras3CJ_Q_T(int& X, int& Y, short& cont, char* Nombre);
void Letras3CJ_U_X(int& X, int& Y, short& cont, char* Nombre);
void Letras3CJ_Y_Z(int& X, int& Y, short& cont, char* Nombre);
void LetrA(int& X, int& Y, short& cont, char* Nombre);
void LetrO(int& X, int& Y, short& cont, char* Nombre);
void Opciones(int& X, int& Y, short& contL, short* ColorFJ);  //  Funci�n Opciones, Env�o: P�rametro por referencia
void Instrucciones(int& X, int& Y);  //  Funci�n Instrucciones, Env�o: P�rametro por referencia
void Salir(int& X, int& Y, int* YFlch, int* XFlch);  //  Funci�n Salir, Env�o: P�rametro por referencia
void Cr�ditos(int& X, int& Y);  //  Funci�n Cr�ditos, Env�o: P�rametro por referencia
void Dados(int& X, int& Y, short* Turno, short* ColorD, short& contL, short& R);
void Mapa(int& X, int& Y, short* Turno, short& contL, short* ColorF, short* ColorFJ, bool Molino, short* ColorA);
void ColorFichas(int& X, int& Y, short& contL);
//  Fin Funciones

using namespace std;  //  Using Namespace Est�ndar
#define cout gameOut

struct Tablero {
	int Color;
};

struct Ficha {
	short Cantidad;
	int Color;
};

typedef struct {
	char Nombre[5];
	short Turno;
	struct Ficha Fichas;
	struct Tablero Tableros;
}tUsuario;

int main() {  //  Main ( Funci�n Principal )
	ios_base::sync_with_stdio(false);
	std::cin.tie(nullptr);
	SetConsoleOutputCP(437);
	SetConsoleCP(437);
	scrHideCursor();
	Molino_v9_20();
	return 0;
}


// ===== Controles compactos =====
enum { TK_UP = 72, TK_DOWN = 80, TK_LEFT = 75, TK_RIGHT = 77, TK_ENTER = 13, TK_ESC = 27 };

static const int BOARD_X[24] = {
	59,59,59,103,103,148,148,148,74,74,74,103,103,133,133,133,88,88,88,103,103,118,118,118
};
static const int BOARD_Y[24] = {
	4,21,38,4,38,4,21,38,10,21,32,10,32,10,21,32,16,21,26,16,26,16,21,26
};
static const int BOARD_NBR[4][24] = {
	{2,0,1,4,12,7,5,6,10,8,9,3,20,15,13,14,18,16,17,11,19,23,21,22},
	{1,2,0,11,3,6,7,5,9,10,8,19,4,14,15,13,17,18,16,20,12,22,23,21},
	{5,6,7,0,2,3,14,4,13,1,15,8,10,11,22,12,21,9,23,16,18,19,17,20},
	{3,9,4,5,7,0,1,2,11,17,12,13,15,8,6,10,19,22,20,21,23,16,14,18}
};
static const int MILL[24][3][2] = {
	{{3,5},{1,2},{8,16}},{{9,17},{0,2},{-1,-1}},{{10,18},{0,1},{4,7}},
	{{0,5},{11,19},{-1,-1}},{{2,7},{12,20},{-1,-1}},{{0,3},{6,7},{13,21}},
	{{5,7},{22,14},{-1,-1}},{{5,6},{2,4},{23,15}},{{11,13},{9,10},{0,16}},
	{{8,10},{1,17},{-1,-1}},{{8,9},{2,18},{12,15}},{{3,19},{8,13},{-1,-1}},
	{{10,15},{20,4},{-1,-1}},{{8,11},{14,15},{21,5}},{{13,15},{22,6},{-1,-1}},
	{{10,12},{13,14},{23,7}},{{0,8},{19,21},{17,18}},{{1,9},{16,18},{-1,-1}},
	{{2,10},{20,23},{16,17}},{{16,21},{11,3},{-1,-1}},{{18,23},{12,4},{-1,-1}},
	{{16,19},{13,5},{22,23}},{{14,6},{21,23},{-1,-1}},{{15,7},{21,22},{18,20}}
};
static const int MOB_N[24][4] = {
	{3,1,-1,-1},{0,9,2,-1},{1,4,-1,-1},{0,11,5,-1},{2,12,7,-1},{3,6,-1,-1},
	{5,14,7,-1},{4,6,-1,-1},{9,11,-1,-1},{1,8,10,17},{9,12,-1,-1},{8,3,19,13},
	{10,20,4,15},{11,14,-1,-1},{22,13,15,6},{12,14,-1,-1},{17,19,-1,-1},{16,18,9,-1},
	{17,20,-1,-1},{16,11,21,-1},{18,12,23,-1},{19,22,-1,-1},{21,23,14,-1},{20,22,-1,-1}
};
static const int NK_X[29] = {
	62,74,86,98,110,122,134,146,62,74,86,98,110,122,134,146,62,74,86,98,110,122,134,146,62,74,42,88,141
};
static const int NK_Y[29] = {
	17,17,17,17,17,17,17,17,23,23,23,23,23,23,23,23,29,29,29,29,29,29,29,29,35,35,42,42,42
};
static const char NK_CH[29] = {
	'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','\b',' ','\n'
};
static const int NK_NBR[4][29] = {
	{24,25,18,19,20,21,22,23,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,24,25,25},
	{8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,2,3,4,5,6,7,26,26,0,4,7},
	{7,0,1,2,3,4,5,6,15,8,9,10,11,12,13,14,23,16,17,18,19,20,21,22,25,24,28,26,27},
	{1,2,3,4,5,6,7,0,9,10,11,12,13,14,15,8,17,18,19,20,21,22,23,16,25,24,27,28,26}
};

static bool contrTodosCero(const short* Contr) {
	for (int i = 0; i < 8; ++i) if (Contr[i] != 0) return false;
	return true;
}
static char leerTecla(const int* ok, int n) {
	char t;
	for (;;) {
		_getch();
		t = (char)_getch();
		for (int i = 0; i < n; ++i) if ((int)(unsigned char)t == ok[i] || t == ok[i]) return t;
	}
}
static int casillaEn(int x, int y) {
	for (int i = 0; i < 24; ++i) if (BOARD_X[i] == x && BOARD_Y[i] == y) return i;
	return -1;
}
static int teclaNombreEn(int x, int y) {
	for (int i = 0; i < 29; ++i) if (NK_X[i] == x && NK_Y[i] == y) return i;
	return -1;
}
static bool ocupada(const short* ColorF, const tUsuario* V, int i) {
	return V[0].Fichas.Color == ColorF[i] || V[1].Fichas.Color == ColorF[i];
}
static bool formaMolino(const short* ColorF, int color, int i) {
	for (int p = 0; p < 3; ++p) {
		int a = MILL[i][p][0], b = MILL[i][p][1];
		if (a < 0) continue;
		if (color == ColorF[a] && color == ColorF[b]) return true;
	}
	return false;
}
static bool esVecinoReal(int src, int dst) {
	for (int k = 0; k < 4; ++k) if (MOB_N[src][k] == dst) return true;
	return false;
}
static bool movilidadBloqueada(const short* ColorF, const tUsuario* V, int i) {
	for (int k = 0; k < 4; ++k) {
		int n = MOB_N[i][k];
		if (n < 0) continue;
		if (!ocupada(ColorF, V, n)) return false;
	}
	return true;
}
static void irVictoria(short* Contr, short& contL) {
	Contr[7] = 0; Contr[1] = 1; contL = 0;
}
static void moverHighlightColorF(int* XFlch, int* YFlch, short* ColorF, const tUsuario* V, int dir) {
	int src = casillaEn(*XFlch, *YFlch); if (src < 0) return;
	int dst = BOARD_NBR[dir][src];
	*XFlch = BOARD_X[dst]; *YFlch = BOARD_Y[dst];
	if (!ocupada(ColorF, V, src)) ColorF[src] = 0;
	if (!ocupada(ColorF, V, dst)) ColorF[dst] = 8;
}
static void moverHighlightColorA(int* XFlch, int* YFlch, short* ColorA, int dir, short mark) {
	int src = casillaEn(*XFlch, *YFlch); if (src < 0) return;
	int dst = BOARD_NBR[dir][src];
	*XFlch = BOARD_X[dst]; *YFlch = BOARD_Y[dst];
	ColorA[src] = 0; ColorA[dst] = mark;
}
static void deslizarPieza(int* XFlch, int* YFlch, short* ColorF, const tUsuario* V,
	int dir, short& Pasos, short& Arriba, short& Abajo, short& Izquierda, short& Derecha) {
	int src = casillaEn(*XFlch, *YFlch); if (src < 0) return;
	int dst = BOARD_NBR[dir][src];
	if (!esVecinoReal(src, dst)) return;
	if (ocupada(ColorF, V, dst)) return;
	bool forward = (Pasos == 1);
	bool reverse = false;
	if (!forward) {
		if (dir == 0) reverse = (Arriba == 1);
		else if (dir == 1) reverse = (Abajo == 1);
		else if (dir == 2) reverse = (Izquierda == 1);
		else reverse = (Derecha == 1);
	}
	if (!forward && !reverse) return;
	int who = -1;
	if (V[0].Fichas.Color == ColorF[src]) who = 0;
	else if (V[1].Fichas.Color == ColorF[src]) who = 1;
	if (who < 0) return;
	*XFlch = BOARD_X[dst]; *YFlch = BOARD_Y[dst];
	ColorF[src] = 0; ColorF[dst] = V[who].Fichas.Color;
	if (forward) {
		Pasos--;
		if (dir == 0) Abajo++; else if (dir == 1) Arriba++; else if (dir == 2) Derecha++; else Izquierda++;
	} else {
		Pasos++;
		if (dir == 0) Arriba--; else if (dir == 1) Abajo--; else if (dir == 2) Izquierda--; else Derecha--;
	}
}
static void volarPieza(int* XFlch, int* YFlch, short* ColorF, const tUsuario* V, int dir, short& Pasos) {
	int src = casillaEn(*XFlch, *YFlch); if (src < 0) return;
	int dst = BOARD_NBR[dir][src];
	if (ocupada(ColorF, V, dst)) return;
	int who = -1;
	if (V[0].Fichas.Color == ColorF[src]) who = 0;
	else if (V[1].Fichas.Color == ColorF[src]) who = 1;
	if (who < 0) return;
	*XFlch = BOARD_X[dst]; *YFlch = BOARD_Y[dst];
	ColorF[src] = 0; ColorF[dst] = V[who].Fichas.Color;
	Pasos--;
}

void Molino_v9_20() {
	// Iteradores
	int i;
	int j;
	int* x, * y;
	// Fin iteradores

	//  Contador de letras
	short cont;
	//  Fin Contador de letras

	//  Contador de Listo
	short contL;
	//  Fin Contador de listo

	//  Contador Turno
	short ContT;
	//  Fin Contador Turno

	//  Tipo de Dato tUsuario
	tUsuario* VJugador;
	VJugador = new tUsuario[2];

	short* Turno;
	Turno = new short[2];

	char* Nombre;
	Nombre = new char[5];
	// Fint Tipo de Dato tUsuario

	//  Tipo de Dato int e *int
	int* XFlch, * YFlch;
	char Tecla;
	//  Fin Tipo de Dato int e *int

	//Color Fichas Jugador
	short* ColorFJ;
	//  Fin Color Fichas Jugador

	//  Color Fichas y cantidad de fichas
	short* ColorA;
	short* ColorF;
	short CantF;
	//  Fin Color Fichas y cantidad de fichas

	//  Tipo de Dato short y *short
	short* Cont;
	short ContD;
	short Pasos;
	short* Contr;
	short* Color;
	int* ColorL;
	short Arriba;
	short Abajo;
	short Izquierda;
	short Derecha;
	//  Color y n�mero de Dado
	short* ColorD;
	short R;
	//  Fin Color y n�mero de Dado

	short Opciones[5];
	short OpcionesS[2];
	short OpcionSeleccionada;
	//  Fin Tipo de Dato short y *short

	//  Guardado
	bool Guardado;
	//  Fin Guardado

	bool Fase1;
	bool Fase2;
	bool Fase3;

	//  Tipo de Dato bool
	bool Repetidor;
	//  Fin Tipo de Dato bool
	bool Deslice;
	bool Molino;
	bool Eleccion;
	bool Vuelo;
	//  C�digo para usar N�mero Aleatorio
	srand(time(NULL));
	//  Fin C�digo para usar N�mero Aleatorio

	//  Inicializar iteradores
	i = 0;
	j = 0;
	//  Fin Inicializar iteradores

	//  VJugador
	VJugador[0].Turno = 0;
	VJugador[1].Turno = 0;
	VJugador[0].Fichas.Color = 0;
	VJugador[1].Fichas.Color = 0;
	VJugador[0].Fichas.Cantidad = 0;
	VJugador[1].Fichas.Cantidad = 0;
	//  Fin VJugador

	//  x y 
	x = new (int);
	y = new (int);

	*x = 210;
	*y = 49;
	//  Fin x y

	//  X_Flecha y Y_Flecha ( COORDENADAS )
	XFlch = new (int);
	YFlch = new (int);

	*XFlch = 83;
	*YFlch = 27;
	//  Fin X_Flecha y Y_Flecha ( COORDENADAS )



	//  Colores y Cantidad de fichas
	ColorF = new short[24];
	ColorA = new short[24];
	for (i = 0; i <= 23; i++) {
		ColorF[i] = 0;
		ColorA[i] = 0;
	}
	//  Fin Colores y Cantidad de fichas

	// Contadores {ContJ=0, ContO=1, ContI=2, ContS=3}
	Cont = new short[4];

	Cont[0] = 0;
	Cont[1] = 0;
	Cont[2] = 0;
	Cont[3] = 0;
	//  Fin Contadores

	//  Contadores
	cont = 0;
	contL = 0;
	ContT = 0;
	Pasos = 0;
	CantF = 0;
	Arriba = 0;
	Abajo = 0;
	Izquierda = 0;
	Derecha = 0;
	//  Fin Contadores

	//  Controladores {ContrJ = 0, ContrO = 1, ContrI = 2, ContrS = 3, ContrR = 4}
	Contr = new short[8];

	Contr[0] = 0;
	Contr[1] = 0;
	Contr[2] = 0;
	Contr[3] = 0;
	Contr[4] = 0;
	Contr[5] = 0;
	Contr[6] = 0;
	Contr[7] = 0;
	//  Fin Controladores

	//  Colores Opciones
	Color = new short[5];

	Color[0] = 3;
	Color[1] = 0;
	Color[2] = 0;
	Color[3] = 0;
	Color[4] = 0;
	//  Fin Colores Opciones

	//  Colores Letras
	ColorL = new int[29];

	for (i = 0; i <= 28; i++) {
		ColorL[i] = 0;
	}

	ColorL[0] = 7;
	//  Fin Colores Letras

	//  Random Dado
	R = 1 + rand() % (6);
	//  Fin Random Dado

	//  Contador de dado
	ContD = 0;
	//  Fin Contador de dado

	//  Color Dados aleatorios
	ColorD = new short[6];

	ColorD[0] = 1 + rand() % (15);
	ColorD[1] = 1 + rand() % (15);
	ColorD[2] = 1 + rand() % (15);
	ColorD[3] = 1 + rand() % (15);
	ColorD[4] = 1 + rand() % (15);
	ColorD[5] = 1 + rand() % (15);
	//  Fin color Dados aleatorios

	ColorFJ = new short[2];
	ColorFJ[0] = 0;
	ColorFJ[1] = 0;

	//  Opciones
	Opciones[0] = 1;
	Opciones[1] = 2;
	Opciones[2] = 3;
	Opciones[3] = 4;
	Opciones[4] = 5;
	//  Fin Opciones

	//  Opciones Salir
	OpcionesS[0] = 1;
	OpcionesS[1] = 2;
	//  Fin Opciones Salir

	Tecla = 0;
	OpcionSeleccionada = 1;

	Guardado = false;
	Fase1 = true;
	Fase2 = false;
	Fase3 = false;
	Deslice = false;
	Eleccion = false;
	Molino = false;
	Vuelo = false;
	//  Repetidor
	Repetidor = true;
	//  Fin Repetidor

	//  Controles de Opciones
	do {
		do {
			scrBeginFrame();
			Men�(x, y, YFlch, XFlch, Color, ColorL, Contr, cont, Nombre, contL, Turno, ColorD, R, ColorF, ColorFJ, Molino, ColorA);
			scrEndFrame();
			Beep(150, 200);

			static const int KEYS_NAV[] = { TK_UP, TK_DOWN, TK_LEFT, TK_RIGHT, TK_ENTER, TK_ESC };
			static const int KEYS_SALIR[] = { TK_LEFT, TK_RIGHT, TK_ENTER, TK_ESC };
			static const int KEYS_COLOR[] = { 49, 50, 51, 52, 53, 54, TK_ESC };

			if (contrTodosCero(Contr)) {
				Tecla = leerTecla(KEYS_NAV, 6);
				switch (Tecla) {
				case TK_UP:
					if (*YFlch == 27 && *XFlch == 83) { *YFlch = 39; OpcionSeleccionada = Opciones[3]; Color[0] = 0; Color[3] = 1; }
					else if (*YFlch == 31 && *XFlch == 74) { *XFlch = 83; *YFlch = 27; OpcionSeleccionada = Opciones[0]; Color[1] = 0; Color[0] = 3; }
					else if (*YFlch == 35 && *XFlch == 59) { *XFlch = 74; *YFlch = 31; OpcionSeleccionada = Opciones[1]; Color[2] = 0; Color[1] = 4; }
					else if (*YFlch == 39 && *XFlch == 83) { *XFlch = 59; *YFlch = 35; OpcionSeleccionada = Opciones[2]; Color[3] = 0; Color[2] = 3; }
					break;
				case TK_DOWN:
					if (*YFlch == 27 && *XFlch == 83) { *XFlch = 74; *YFlch = 31; OpcionSeleccionada = Opciones[1]; Color[0] = 0; Color[1] = 4; }
					else if (*YFlch == 31 && *XFlch == 74) { *XFlch = 59; *YFlch = 35; OpcionSeleccionada = Opciones[2]; Color[1] = 0; Color[2] = 3; }
					else if (*YFlch == 35 && *XFlch == 59) { *XFlch = 83; *YFlch = 39; OpcionSeleccionada = Opciones[3]; Color[2] = 0; Color[3] = 1; }
					else if (*YFlch == 39 && *XFlch == 83) { *YFlch = 27; OpcionSeleccionada = Opciones[0]; Color[3] = 0; Color[0] = 3; }
					break;
				case TK_LEFT:
					if (*YFlch == 27 && *XFlch == 83) { *XFlch = 1; *YFlch = 1; OpcionSeleccionada = Opciones[4]; Cont[0]++; Color[0] = 0; Color[4] = 1 + rand() % 13; }
					else if (*YFlch == 31 && *XFlch == 74) { *XFlch = 1; *YFlch = 1; OpcionSeleccionada = Opciones[4]; Cont[1]++; Color[1] = 0; Color[4] = 1 + rand() % 13; }
					else if (*YFlch == 35 && *XFlch == 59) { *XFlch = 1; *YFlch = 1; OpcionSeleccionada = Opciones[4]; Cont[2]++; Color[2] = 0; Color[4] = 1 + rand() % 13; }
					else if (*YFlch == 39 && *XFlch == 83) { *XFlch = 1; *YFlch = 1; OpcionSeleccionada = Opciones[4]; Cont[3]++; Color[3] = 0; Color[4] = 1 + rand() % 13; }
					break;
				case TK_RIGHT:
					if (*YFlch == 1 && *XFlch == 1 && Cont[0] == 1) { *XFlch = 83; *YFlch = 27; OpcionSeleccionada = Opciones[0]; Cont[0]--; Color[4] = 0; Color[0] = 3; }
					else if (*YFlch == 1 && *XFlch == 1 && Cont[1] == 1) { *XFlch = 74; *YFlch = 31; OpcionSeleccionada = Opciones[1]; Cont[1]--; Color[4] = 0; Color[1] = 4; }
					else if (*YFlch == 1 && *XFlch == 1 && Cont[2] == 1) { *XFlch = 59; *YFlch = 35; OpcionSeleccionada = Opciones[2]; Cont[2]--; Color[4] = 0; Color[2] = 3; }
					else if (*YFlch == 1 && *XFlch == 1 && Cont[3] == 1) { *XFlch = 83; *YFlch = 39; OpcionSeleccionada = Opciones[3]; Cont[3]--; Color[4] = 0; Color[3] = 1; }
					break;
				case TK_ENTER:
					if (OpcionSeleccionada == 1) {
						if (Guardado == true) { *XFlch = 59; *YFlch = 4; Contr[7]++; }
						else { *XFlch = 62; *YFlch = 17; Contr[0]++; ColorL[0] = 7; }
					}
					else if (OpcionSeleccionada == 2) Contr[1]++;
					else if (OpcionSeleccionada == 3) Contr[2]++;
					else if (OpcionSeleccionada == 4) { *XFlch = 69; *YFlch = 32; OpcionSeleccionada = OpcionesS[0]; Contr[3]++; }
					else if (OpcionSeleccionada == 5) Contr[4]++;
					break;
				case TK_ESC:
					Contr[3]++; *XFlch = 69; *YFlch = 32; OpcionSeleccionada = OpcionesS[0];
					Color[3] = 1; Color[4] = Color[0] = Color[2] = Color[1] = 0;
					Cont[0] = Cont[1] = Cont[2] = Cont[3] = 0;
					break;
				}
			}
			else if (Contr[7] == 1) {
				Tecla = leerTecla(KEYS_NAV, 6);
				int dir = (Tecla == TK_UP) ? 0 : (Tecla == TK_DOWN) ? 1 : (Tecla == TK_LEFT) ? 2 : (Tecla == TK_RIGHT) ? 3 : -1;
				if (dir >= 0) {
					if (Fase1 == true && Molino == false) moverHighlightColorF(XFlch, YFlch, ColorF, VJugador, dir);
					else if (Fase1 == true && Molino == true) moverHighlightColorA(XFlch, YFlch, ColorA, dir, -4);
					else if (Fase2 == true && Eleccion == true) moverHighlightColorA(XFlch, YFlch, ColorA, dir, 6);
					else if (Fase2 == true && Deslice == true) deslizarPieza(XFlch, YFlch, ColorF, VJugador, dir, Pasos, Arriba, Abajo, Izquierda, Derecha);
					else if (Fase2 == true && Molino == true) moverHighlightColorA(XFlch, YFlch, ColorA, dir, -4);
					else if (Fase3 == true && Eleccion == true) moverHighlightColorA(XFlch, YFlch, ColorA, dir, 6);
					else if (Fase3 == true && Vuelo == true) volarPieza(XFlch, YFlch, ColorF, VJugador, dir, Pasos);
					else if (Fase3 == true && Deslice == true) deslizarPieza(XFlch, YFlch, ColorF, VJugador, dir, Pasos, Arriba, Abajo, Izquierda, Derecha);
					else if (Fase3 == true && Molino == true) moverHighlightColorA(XFlch, YFlch, ColorA, dir, -4);
				}
				else if (Tecla == TK_ENTER) {
					int ci = casillaEn(*XFlch, *YFlch);
					if (ci >= 0) {
						if (Fase1 == true && Molino == false) {
							int pl = (ContT == 0) ? 0 : 1;
							if (!(VJugador[0].Fichas.Color == ColorF[ci] || VJugador[1].Fichas.Color == ColorF[ci]) && ColorF[ci] == 8) {
								ColorF[ci] = VJugador[pl].Fichas.Color;
								if (pl == 0) { contL++; ContT++; } else { contL--; ContT--; }
								VJugador[pl].Fichas.Cantidad++;
								CantF++;
								if (formaMolino(ColorF, VJugador[pl].Fichas.Color, ci)) {
									if (pl == 0) { contL--; ContT--; } else { contL = 1; ContT = 1; }
									ColorA[ci] = -4; Molino = true;
								}
								if (CantF == 18 && Molino == false) {
									Fase1 = false; Fase2 = true; Eleccion = true; ColorA[ci] = 6;
								}
							}
						}
						else if (Fase1 == true && Molino == true) {
							int atk = (ContT == 0) ? 0 : 1;
							int vic = 1 - atk;
							if (VJugador[vic].Fichas.Color == ColorF[ci]) {
								bool enM = formaMolino(ColorF, VJugador[vic].Fichas.Color, ci);
								bool ok = (!enM) || (VJugador[vic].Fichas.Cantidad == 3);
								if (ok) {
									ColorF[ci] = 8; ColorA[ci] = 0;
									if (atk == 0) { ContT++; contL++; } else { ContT--; contL--; }
									VJugador[vic].Fichas.Cantidad--;
									if (CantF == 18 && ColorF[ci] == 8) { ColorF[ci] = 0; ColorA[ci] = 6; }
									Molino = false;
									if (CantF == 18) { Fase1 = false; Fase2 = true; Eleccion = true; }
								}
							}
						}
						else if (Fase2 == true && Eleccion == true) {
							int pl = (ContT == 0) ? 0 : 1;
							if (VJugador[pl].Fichas.Color == ColorF[ci] && !movilidadBloqueada(ColorF, VJugador, ci)) {
								ColorA[ci] = 0; Eleccion = false; Deslice = true; Pasos = 1;
								Arriba = Abajo = Izquierda = Derecha = 0;
							}
						}
						else if (Fase2 == true && Deslice == true) {
							if (Pasos != 1) {
								if (formaMolino(ColorF, VJugador[(ContT == 0) ? 0 : 1].Fichas.Color, ci)) {
									ColorA[ci] = -4; Molino = true; Deslice = false;
								} else {
									ColorA[ci] = 6; contL += (ContT == 0) ? 1 : -1;
									if (ContT == 0) ContT++; else ContT--;
									Eleccion = true; Deslice = false;
								}
							}
						}
						else if (Fase2 == true && Molino == true) {
							int atk = (ContT == 0) ? 0 : 1;
							int vic = 1 - atk;
							if (VJugador[vic].Fichas.Color == ColorF[ci]) {
								bool enM = formaMolino(ColorF, VJugador[vic].Fichas.Color, ci);
								if (!enM) {
									ColorF[ci] = 0; ColorA[ci] = 6;
									if (atk == 0) { ContT++; contL++; } else { ContT--; contL--; }
									VJugador[vic].Fichas.Cantidad--;
									Molino = false; Eleccion = true;
									if (VJugador[vic].Fichas.Cantidad == 3) { Fase2 = false; Fase3 = true; }
								}
							}
						}
						else if (Fase3 == true && Eleccion == true) {
							int pl = (ContT == 0) ? 0 : 1;
							if (VJugador[pl].Fichas.Color == ColorF[ci] && !movilidadBloqueada(ColorF, VJugador, ci)) {
								ColorA[ci] = 0; Eleccion = false; Pasos = 1;
								if (VJugador[pl].Fichas.Cantidad == 3) Vuelo = true;
								else if (VJugador[pl].Fichas.Cantidad > 3) {
									Deslice = true; Arriba = Abajo = Izquierda = Derecha = 0;
								}
							}
						}
						else if (Fase3 == true && (Deslice == true || Vuelo == true)) {
							if (Pasos != 1) {
								int pl = (ContT == 0) ? 0 : 1;
								if (formaMolino(ColorF, VJugador[pl].Fichas.Color, ci)) {
									ColorA[ci] = -4; Molino = true; Deslice = false; Vuelo = false;
								} else {
									ColorA[ci] = 6;
									if (pl == 0) { contL++; ContT++; } else { contL--; ContT--; }
									Eleccion = true; Deslice = false; Vuelo = false;
								}
							}
						}
						else if (Fase3 == true && Molino == true) {
							int atk = (ContT == 0) ? 0 : 1;
							int vic = 1 - atk;
							if (VJugador[vic].Fichas.Color == ColorF[ci]) {
								bool enM = formaMolino(ColorF, VJugador[vic].Fichas.Color, ci);
								bool ok = (!enM) || (VJugador[vic].Fichas.Cantidad == 3);
								if (ok) {
									ColorF[ci] = 0; ColorA[ci] = 6;
									if (atk == 0) { ContT++; contL++; } else { ContT--; contL--; }
									VJugador[vic].Fichas.Cantidad--;
									Molino = false; Eleccion = true; Vuelo = false;
									if (VJugador[vic].Fichas.Cantidad == 2) irVictoria(Contr, contL);
								}
							}
						}
					}
				}
				else if (Tecla == TK_ESC) {
					*XFlch = 83; *YFlch = 27; Contr[7]--; Guardado = true;
				}
			}
			else if (Contr[6] == 1) {
				if (contL == 0) {
					for (i = 0; i <= 4; i++) {
						R = 1 + rand() % 6;
						if (ContD == 3) Turno[0] = R;
						ContD++;
						for (j = 0; j <= 5; j++) ColorD[j] = 1 + rand() % 15;
						break;
					}
					if (ContD == 5) {
						getch(); contL++; ContD = 0; VJugador[0].Turno = Turno[0];
					}
				}
				else if (contL == 1) {
					for (i = 0; i <= 4; i++) {
						R = 1 + rand() % 6;
						if (ContD == 3) { Turno[1] = R; VJugador[1].Turno = Turno[1]; }
						ContD++;
						for (j = 0; j <= 5; j++) ColorD[j] = 1 + rand() % 15;
						break;
					}
					if (ContD == 5) {
						getch();
						if (Turno[0] == Turno[1]) { ContD = 0; contL = 0; }
						else {
							*XFlch = 59; *YFlch = 4; ColorF[0] = 8; contL = 0; ContD = 0;
							if (Turno[0] > Turno[1]) { VJugador[0].Turno = 1; VJugador[1].Turno = 2; }
							else { VJugador[0].Turno = 2; VJugador[1].Turno = 1; contL++; ContT++; }
							Contr[6] = 0; Contr[7] = 1;
						}
					}
				}
			}
			else if (Contr[5] == 1) {
				Tecla = leerTecla(KEYS_COLOR, 7);
				if (Tecla >= 49 && Tecla <= 54) {
					int col = Tecla - 48;
					if (contL == 0) { VJugador[0].Fichas.Color = col; ColorFJ[0] = col; contL++; }
					else if (contL == 1) { VJugador[1].Fichas.Color = col; ColorFJ[1] = col; contL--; Contr[5]--; Contr[6] = 1; }
				}
				else if (Tecla == TK_ESC) { Contr[5]--; *XFlch = 83; *YFlch = 27; }
			}
			else if (Contr[4] == 1) {
				Tecla = leerTecla(KEYS_NAV, 6);
				(void)Tecla;
			}
			else if (Contr[3] == 1) {
				Tecla = leerTecla(KEYS_SALIR, 4);
				if (Tecla == TK_LEFT || Tecla == TK_RIGHT) {
					if (*XFlch == 69 && *YFlch == 32) { *XFlch = 116; OpcionSeleccionada = OpcionesS[1]; }
					else if (*XFlch == 116 && *YFlch == 32) { *XFlch = 69; OpcionSeleccionada = OpcionesS[0]; }
				}
				else if (Tecla == TK_ENTER) {
					if (OpcionSeleccionada == 1) Repetidor = false;
					else if (OpcionSeleccionada == 2) { Contr[3]--; *XFlch = 83; *YFlch = 39; OpcionSeleccionada = Opciones[3]; }
				}
				else if (Tecla == TK_ESC) {
					Contr[3]--; *XFlch = 83; *YFlch = 39; OpcionSeleccionada = Opciones[3];
				}
			}
			else if (Contr[2] == 1) {
				Tecla = leerTecla(KEYS_NAV, 6);
				if (Tecla == TK_ESC) { *XFlch = 83; *YFlch = 27; Contr[2]--; }
			}
			else if (Contr[1] == 1) {
				Tecla = leerTecla(KEYS_NAV, 6);
				if (Tecla == TK_ESC) { *XFlch = 83; *YFlch = 27; Color[1] = 0; Color[0] = 3; Contr[1]--; }
			}
			else if (Contr[0] == 1) {
				Tecla = leerTecla(KEYS_NAV, 6);
				int nk = teclaNombreEn(*XFlch, *YFlch);
				int ndir = (Tecla == TK_UP) ? 0 : (Tecla == TK_DOWN) ? 1 : (Tecla == TK_LEFT) ? 2 : (Tecla == TK_RIGHT) ? 3 : -1;
				if (ndir >= 0 && nk >= 0) {
					int dst = NK_NBR[ndir][nk];
					ColorL[nk] = 0;
					ColorL[dst] = (dst == 26) ? 4 : 7;
					*XFlch = NK_X[dst]; *YFlch = NK_Y[dst];
				}
				else if (Tecla == TK_ENTER && nk >= 0) {
					if (nk <= 25 && cont < 5) {
						Nombre[cont] = NK_CH[nk];
						cont++;
					}
					else if (nk == 26 && cont > 0) {
						Nombre[cont - 1] = ' ';
						cont--;
					}
					else if (nk == 27 && cont >= 1 && cont < 5) {
						Nombre[cont] = ' ';
						cont++;
					}
					else if (nk == 28 && cont >= 1 && cont <= 4) {
						if (cont <= 3) {
							Nombre[cont] = ' ';
							if (cont <= 2) Nombre[cont + 1] = ' ';
							if (cont == 1) Nombre[cont + 2] = ' ';
						}
						int p = (contL == 0) ? 0 : 1;
						VJugador[p].Nombre[0] = Nombre[0];
						VJugador[p].Nombre[1] = Nombre[1];
						VJugador[p].Nombre[2] = Nombre[2];
						VJugador[p].Nombre[3] = Nombre[3];
						if (contL == 0) contL++;
						else { contL = 0; Contr[0] = 0; Contr[5]++; }
						cont = 0; *XFlch = 62; *YFlch = 17; ColorL[0] = 7; ColorL[28] = 0;
					}
				}
				else if (Tecla == TK_ESC) {
					Contr[0] = 0; *XFlch = 83; *YFlch = 27; OpcionSeleccionada = Opciones[0];
					for (i = 0; i <= 28; i++) ColorL[i] = 0;
				}
			}
		} while (Tecla != 72 && Tecla != 80 && Tecla != 75 && Tecla != 77 && Tecla != 13 && Tecla != 27);
	} while (Repetidor);
	//  Fin Controles de Opciones


	//  Se libera espacio de memoria
	delete[] VJugador;
	VJugador = NULL;

	delete x;
	delete y;
	x = NULL;
	y = NULL;

	delete XFlch;
	delete YFlch;
	XFlch = NULL;
	YFlch = NULL;

	delete[] Nombre;
	Nombre = NULL;

	delete[] Turno;
	Turno = NULL;

	delete[] VJugador;
	VJugador = NULL;

	delete[] Cont;
	Cont = NULL;

	delete[] ColorF;
	ColorF = NULL;

	delete[] ColorA;
	ColorA = NULL;

	delete[] ColorFJ;
	ColorFJ = NULL;

	delete[] Contr;
	Contr = NULL;

	delete[] Color;
	Color = NULL;

	delete[] ColorD;
	ColorD = NULL;

	delete[] ColorL;
	ColorL = NULL;
	//  Fin Se libera el espacio reservado anteriormente
}

void Men�(int* x, int* y, int* YFlch, int* XFlch, short* Color, int* ColorL, short* Contr, short& cont, char* Nombre, short& contL, short* Turno, short* ColorD, short& R, short* ColorF, short* ColorFJ, bool Molino, short* ColorA) {
	int X, Y;

	for (Y = 1; Y <= *y; Y++) {

		for (X = 1; X <= *x; X++) {
			gScrX = X;
			gScrY = Y;

			color(hConsole, 3);  //  Color del Marco ( Azul )

			if (Y == 1 && X == 1) {  //  Marco
				cout << char(201);
			}
			else
				if (Y == 1 && X == 210) {
					cout << char(187);
				}
				else
					if (Y == 49 && X == 1) {
						cout << char(200);
					}
					else
						if (Y == 49 && X == 210) {
							cout << char(188);
						}
						else
							if (Y == 1 || Y == 49) {
								cout << char(205);
							}
							else
								if (X == 1 || X == 210) {
									cout << char(186);
								}
								else  //  Fin Marco
									if ((X == 4 || X == 5 || X == 6 || X == 7 || X == 8 || X == 9 || X == 10 || X == 11 || X == 12 || X == 13 || X == 14 || X == 15 || X == 16 || X == 17 || X == 18 || X == 19 || X == 20 || X == 21 || X == 22) && (Y == 2 || Y == 3 || Y == 4 || Y == 5 || Y == 6 || Y == 7 || Y == 8 || Y == 9 || Y == 10 || Y == 11 || Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32 || Y == 33 || Y == 34 || Y == 35 || Y == 36 || Y == 37 || Y == 38 || Y == 39 || Y == 40 || Y == 41 || Y == 42 || Y == 43 || Y == 44 || Y == 45 || Y == 46 || Y == 47 || Y == 48)) {  //  Dibuja Torre Izquierda
										Dibuja_Torre_Izquierda(X, Y, Color);
									}
									else  //  Fin Dibuja Torre Izquierda
										if ((X == 207 || X == 206 || X == 205 || X == 204 || X == 203 || X == 202 || X == 201 || X == 200 || X == 199 || X == 198 || X == 197 || X == 196 || X == 195 || X == 194 || X == 193 || X == 192 || X == 191 || X == 190 || X == 189) && (Y == 2 || Y == 3 || Y == 4 || Y == 5 || Y == 6 || Y == 7 || Y == 8 || Y == 9 || Y == 10 || Y == 11 || Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32 || Y == 33 || Y == 34 || Y == 35 || Y == 36 || Y == 37 || Y == 38 || Y == 39 || Y == 40 || Y == 41 || Y == 42 || Y == 43 || Y == 44 || Y == 45 || Y == 46 || Y == 47 || Y == 48)) {  //  Dibuja Torre Derecha
											Dibuja_Torre_Derecha(X, Y);
										}
										else  //  Fin Dibuja Torre Derecha
											if ((Y == 32 || Y == 33 || Y == 34 || Y == 35 || Y == 36 || Y == 37 || Y == 38 || Y == 39 || Y == 40) && (X == 23 || X == 24 || X == 25 || X == 26 || X == 27 || X == 28 || X == 29 || X == 30 || X == 31 || X == 32 || X == 33 || X == 34 || X == 35 || X == 36 || X == 37 || X == 38)) {  //  Dibuja Cubito
												color(hConsole, 14 - Color[4]);
												Dibuja_Cubito(X, Y);
											}
											else  //  Fin Dibuja Cubito
												if ((Y == 32 || Y == 33 || Y == 34 || Y == 35 || Y == 36 || Y == 37 || Y == 38 || Y == 39 || Y == 40) && (X == 188 || X == 187 || X == 186 || X == 185 || X == 184 || X == 183 || X == 182 || X == 181 || X == 180 || X == 179 || X == 178 || X == 177 || X == 176 || X == 175 || X == 174 || X == 173 || X == 172 || X == 171)) {  //  Dibuja Ventilador
													Dibuja_Ventilador(X, Y);
												}  //  Fin Dibuja Ventilador
												else  //  Menu principal
													if (Contr[0] == 0 && Contr[1] == 0 && Contr[2] == 0 && Contr[3] == 0 && Contr[4] == 0 && Contr[5] == 0 && Contr[6] == 0 && Contr[7] == 0) {  //  Controladores 
														if ((X == 77 || X == 76 || X == 75 || X == 74 || X == 73 || X == 72 || X == 71 || X == 70 || X == 69 || X == 68 || X == 67 || X == 66 || X == 65 || X == 64 || X == 63 || X == 62 || X == 61 || X == 60 || X == 59 || X == 58 || X == 57 || X == 56 || X == 55 || X == 54 || X == 53 || X == 52 || X == 51 || X == 50 || X == 49 || X == 48 || X == 47 || X == 46 || X == 45 || X == 44 || X == 43 || X == 42 || X == 41 || X == 40 || X == 39) && (Y == 23 || Y == 22 || Y == 21 || Y == 20 || Y == 19 || Y == 18 || Y == 17 || Y == 16 || Y == 15 || Y == 14 || Y == 13 || Y == 12 || Y == 11 || Y == 10 || Y == 9 || Y == 8 || Y == 7)) {
															Dibuja_L_Titulo(X, Y);
														}
														else
															if ((X == 78 || X == 79 || X == 80 || X == 81 || X == 82 || X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113) && (Y == 23 || Y == 22 || Y == 21 || Y == 20 || Y == 19 || Y == 18 || Y == 17 || Y == 16 || Y == 15 || Y == 14 || Y == 13 || Y == 12 || Y == 11 || Y == 10 || Y == 9 || Y == 8 || Y == 7)) {
																Dibuja_E_Titulo(X, Y);
															}
															else
																if ((X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130 || X == 131 || X == 132 || X == 133 || X == 134 || X == 135 || X == 136 || X == 137 || X == 138 || X == 139 || X == 140 || X == 141 || X == 142 || X == 143 || X == 144 || X == 145 || X == 146 || X == 147 || X == 148 || X == 149 || X == 150 || X == 151 || X == 152 || X == 153 || X == 154 || X == 155 || X == 156 || X == 157 || X == 158 || X == 159 || X == 160 || X == 161 || X == 162 || X == 163 || X == 164 || X == 165 || X == 166 || X == 167 || X == 168 || X == 169 || X == 170 || X == 171 || X == 172 || X == 173 || X == 174) && (Y == 8 || Y == 9 || Y == 10 || Y == 11 || Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23)) {
																	Dibuja_I_Titulo(X, Y);
																}
																else
																	if (Y == 45 || Y == 46 || Y == 47) {  //  C 2022 Programaci�n I
																		C_2022_Programaci�n1(X, Y);
																	}
																	else  //  Fin C 2022 Programaci�n I	
																		if ((Y == 27 || Y == 28 || Y == 29) && (X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119)) {  //  Dibuja Opci�n Jugar
																			color(hConsole, 7 - Color[0]);
																			Dibuja_OJugar(X, Y);
																		}
																		else  //  Fin Dibuja Opci�n Jugar
																			if ((Y == 31 || Y == 32 || Y == 33) && (X == 82 || X == 83 || X == 84 || X == 85 || X == 86 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128)) {  //  Dibuja Opci�n Opciones
																				color(hConsole, 7 + Color[1]);
																				Dibuja_OOpciones(X, Y);
																			}
																			else  //  Fin Dibuja Opci�n Opciones
																				if ((Y == 35 || Y == 36 || Y == 37) && (X == 67 || X == 68 || X == 69 || X == 70 || X == 71 || X == 73 || X == 74 || X == 75 || X == 76 || X == 77 || X == 79 || X == 80 || X == 81 || X == 82 || X == 83 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 127 || X == 128 || X == 129 || X == 130 || X == 131 || X == 133 || X == 134 || X == 135 || X == 136 || X == 137 || X == 139 || X == 140 || X == 141 || X == 142 || X == 143)) {  //  Dibuja Opci�n Instrucciones
																					color(hConsole, 7 + Color[2]);
																					Dibuja_OInstrucciones(X, Y);
																				}
																				else  //  Fin Dibuja Opci�n Instrucciones
																					if ((Y == 39 || Y == 40 || Y == 41) && (X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119)) {  //  Dibuja Opci�n Salir
																						color(hConsole, 7 - Color[3]);
																						Dibuja_OSalir(X, Y);
																					}
																					else  //  Fin Dibuja Opci�n Salir
																						if ((Y == 27 || Y == 28 || Y == 29 || Y == 31 || Y == 32 || Y == 33 || Y == 35 || Y == 36 || Y == 37 || Y == 39 || Y == 40 || Y == 41) && (X == 83 || X == 82 || X == 93 || X == 94 || X == 127 || X == 128 || X == 74 || X == 73 || X == 136 || X == 137 || X == 59 || X == 58 || X == 151 || X == 152)) {  //  Dibuja Flecha
																							Dibuja_Flechita(X, Y, YFlch, XFlch, ColorL);
																						}
																						else  //  Fin Dibuja Flecha
																							cout << " ";
													}
													else//  Fin Menu principal
														if (Contr[0] == 1) {  //  Controlador_Jugar
															if ((X == 42 || X == 43 || X == 44 || X == 45 || X == 46 || X == 47 || X == 48 || X == 49 || X == 50 || X == 51 || X == 52 || X == 53 || X == 54 || X == 55 || X == 56 || X == 57 || X == 58 || X == 59 || X == 60 || X == 61 || X == 62 || X == 63 || X == 64 || X == 65 || X == 66 || X == 67 || X == 68 || X == 69 || X == 70 || X == 71 || X == 72 || X == 73 || X == 74 || X == 75 || X == 76 || X == 77 || X == 78 || X == 79 || X == 80 || X == 81 || X == 82 || X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130 || X == 131 || X == 132 || X == 133 || X == 134 || X == 135 || X == 136 || X == 137 || X == 138 || X == 139 || X == 140 || X == 141 || X == 142 || X == 143 || X == 144 || X == 145 || X == 146 || X == 147 || X == 148 || X == 149 || X == 150 || X == 151 || X == 152 || X == 153 || X == 154 || X == 155 || X == 156 || X == 157 || X == 158 || X == 159 || X == 160 || X == 161 || X == 162 || X == 163 || X == 164 || X == 165 || X == 166 || X == 167 || X == 168 || X == 169 || X == 170 || X == 171 || X == 172 || X == 173 || X == 174 || X == 175) && (Y == 44 || Y == 43 || Y == 42 || Y == 41 || Y == 40 || Y == 39 || Y == 38 || Y == 37 || Y == 36 || Y == 35 || Y == 34 || Y == 33 || Y == 32 || Y == 31 || Y == 30 || Y == 29 || Y == 28 || Y == 27 || Y == 26 || Y == 25 || Y == 24 || Y == 23 || Y == 22 || Y == 21 || Y == 20 || Y == 19 || Y == 18 || Y == 17 || Y == 16 || Y == 15 || Y == 14 || Y == 13 || Y == 12 || Y == 11 || Y == 10 || Y == 9 || Y == 8 || Y == 7)) {
																Jugar(X, Y, ColorL, cont, Nombre, contL);
															}
															else
																cout << " ";
														}
														else  //  Fin Controlador_Jugar
															if (Contr[1] == 1) {  //  Controlador_Opciones
																Opciones(X, Y, contL, ColorFJ);
															}
															else  //  Fin Controlador_Opciones
																if (Contr[2] == 1) {  //  Controlador_Instrucciones
																	Instrucciones(X, Y);
																}
																else  //  Fin Controlador_Instrucciones
																	if (Contr[3] == 1) {  //  Controlador_Salir
																		if ((X == 51 || X == 52 || X == 53 || X == 54 || X == 55 || X == 56 || X == 57 || X == 58 || X == 59 || X == 60 || X == 61 || X == 62 || X == 63 || X == 64 || X == 65 || X == 66 || X == 67 || X == 68 || X == 69 || X == 70 || X == 71 || X == 72 || X == 73 || X == 74 || X == 75 || X == 76 || X == 77 || X == 78 || X == 79 || X == 80 || X == 81 || X == 82 || X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130 || X == 131 || X == 132 || X == 133 || X == 134 || X == 135 || X == 136 || X == 137 || X == 138 || X == 139 || X == 140 || X == 141 || X == 142 || X == 143 || X == 144 || X == 145 || X == 146 || X == 147 || X == 148 || X == 149 || X == 150 || X == 151 || X == 152 || X == 153 || X == 154 || X == 155 || X == 156 || X == 157 || X == 158 || X == 159 || X == 160) && (Y == 10 || Y == 11 || Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32 || Y == 33 || Y == 34 || Y == 35 || Y == 36 || Y == 37 || Y == 38 || Y == 39 || Y == 40)) {
																			Dibuja_Flechita(X, Y, YFlch, XFlch, ColorL);
																			Salir(X, Y, YFlch, XFlch);
																		}
																		else
																			cout << " ";
																	}
																	else  //  Fin Controlador_Salir
																		if (Contr[4] == 1) {  //  Controlador_Cr�ditos
																			Cr�ditos(X, Y);
																		}
																		else  //  Fin Controlador_Cr�ditos
																			if (Contr[5] == 1) {
																				ColorFichas(X, Y, contL);
																			}
																			else
																				if (Contr[6] == 1) {
																					Dados(X, Y, Turno, ColorD, contL, R);
																				}
																				else
																					if (Contr[7] == 1) {
																						if ((X == 42 || X == 43 || X == 44 || X == 45 || X == 46 || X == 47 || X == 48 || X == 49 || X == 50 || X == 51 || X == 52 || X == 53 || X == 54 || X == 55 || X == 56 || X == 57 || X == 58 || X == 59 || X == 60 || X == 61 || X == 62 || X == 63 || X == 64 || X == 65 || X == 66 || X == 67 || X == 68 || X == 69 || X == 70 || X == 71 || X == 72 || X == 73 || X == 74 || X == 75 || X == 76 || X == 77 || X == 78 || X == 79 || X == 80 || X == 81 || X == 82 || X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130 || X == 131 || X == 132 || X == 133 || X == 134 || X == 135 || X == 136 || X == 137 || X == 138 || X == 139 || X == 140 || X == 141 || X == 142 || X == 143 || X == 144 || X == 145 || X == 146 || X == 147 || X == 148 || X == 149 || X == 150 || X == 151 || X == 152 || X == 153 || X == 154 || X == 155 || X == 156 || X == 157 || X == 158 || X == 159 || X == 160 || X == 161 || X == 162 || X == 163 || X == 164 || X == 165 || X == 166 || X == 167 || X == 168 || X == 169 || X == 170 || X == 171 || X == 172 || X == 173 || X == 174 || X == 175) && (Y == 49 || Y == 48 || Y == 47 || Y == 46 || Y == 45 || Y == 44 || Y == 43 || Y == 42 || Y == 41 || Y == 40 || Y == 39 || Y == 38 || Y == 37 || Y == 36 || Y == 35 || Y == 34 || Y == 33 || Y == 32 || Y == 31 || Y == 30 || Y == 29 || Y == 28 || Y == 27 || Y == 26 || Y == 25 || Y == 24 || Y == 23 || Y == 22 || Y == 21 || Y == 20 || Y == 19 || Y == 18 || Y == 17 || Y == 16 || Y == 15 || Y == 14 || Y == 13 || Y == 12 || Y == 11 || Y == 10 || Y == 9 || Y == 8 || Y == 7 || Y == 6 || Y == 5 || Y == 4 || Y == 3)) {
																							Mapa(X, Y, Turno, contL, ColorF, ColorFJ, Molino, ColorA);
																						}
																						else
																							cout << " ";
																					}
																					else
																						cout << " ";
		}
		cout << endl;
	}
}

void ColorFichas(int& X, int& Y, short& contL) {
	if (contL == 0) {
		if ((X == 114 || X == 115 || X == 116) && (Y == 20 || Y == 21)) {
			color(hConsole, 6);
			cout << char(219);
		}
		else
			if (X == 115 && Y == 23) {
				cout << "6";
			}
			else
				if ((X == 109 || X == 110 || X == 111) && (Y == 20 || Y == 21)) {
					color(hConsole, 5);
					cout << char(219);
				}
				else
					if (X == 110 && Y == 23) {
						cout << "5";
					}
					else
						if ((X == 104 || X == 105 || X == 106) && (Y == 20 || Y == 21)) {
							color(hConsole, 4);
							cout << char(219);
						}
						else
							if (X == 105 && Y == 23) {
								cout << "4";
							}
							else
								if ((X == 99 || X == 100 || X == 101) && (Y == 20 || Y == 21)) {
									color(hConsole, 3);
									cout << char(219);
								}
								else
									if (X == 100 && Y == 23) {
										cout << "3";
									}
									else
										if ((X == 94 || X == 95 || X == 96) && (Y == 20 || Y == 21)) {
											color(hConsole, 2);
											cout << char(219);
										}
										else
											if (X == 95 && Y == 23) {
												cout << "2";
											}
											else
												if ((X == 89 || X == 90 || X == 91) && (Y == 20 || Y == 21)) {
													color(hConsole, 1);
													cout << char(219);
												}
												else
													if (X == 90 && Y == 23) {
														cout << "1";
													}
													else
														if ((X == 118 || X == 119 || X == 120 || X == 121 || X == 122) && (Y == 44 || Y == 45 || Y == 46)) {
															color(hConsole, 7);
															if ((X == 118) && Y == 44) {
																cout << " ";
															}
															else
																if ((X == 118) && Y == 45) {
																	cout << char(220);
																}
																else
																	if ((X == 118) && Y == 46) {
																		cout << " ";
																	}
																	else
																		if ((X == 119 || X == 120 || X == 121) && Y == 44) {
																			cout << " ";
																		}
																		else
																			if ((X == 119 || X == 120 || X == 121) && Y == 45) {
																				cout << " ";
																			}
																			else
																				if ((X == 119 || X == 120 || X == 121) && Y == 46) {
																					cout << char(223);
																				}
																				else
																					if (X == 122 && Y == 44) {
																						cout << char(219);
																					}
																					else
																						if (X == 122 && Y == 45) {
																							cout << char(219);
																						}
																						else
																							if (X == 122 && Y == 46) {
																								cout << " ";
																							}
														}
														else
															if ((X == 124 || X == 125 || X == 126 || X == 127 || X == 128) && (Y == 44 || Y == 45 || Y == 46)) {
																color(hConsole, 7);
																if ((X == 124 || X == 127 || X == 128) && Y == 44) {
																	cout << " ";
																}
																else
																	if ((X == 124 || X == 127 || X == 128) && Y == 45) {
																		cout << " ";
																	}
																	else
																		if ((X == 124 || X == 127 || X == 128) && Y == 46) {
																			cout << " ";
																		}
																		else
																			if (X == 125 && Y == 44) {
																				cout << char(223);
																			}
																			else
																				if (X == 125 && Y == 45) {
																					cout << " ";
																				}
																				else
																					if (X == 125 && Y == 46) {
																						cout << " ";
																					}
																					else
																						if (X == 126 && Y == 44) {
																							cout << char(219);
																						}
																						else
																							if (X == 126 && Y == 45) {
																								cout << char(219);
																							}
																							else
																								if (X == 126 && Y == 46) {
																									cout << char(223);
																								}
															}
															else
																cout << " ";
	}
	else
		if (contL == 1) {
			if ((X == 114 || X == 115 || X == 116) && (Y == 20 || Y == 21)) {
				color(hConsole, 6);
				cout << char(219);
			}
			else
				if (X == 115 && Y == 23) {
					cout << "6";
				}
				else
					if ((X == 109 || X == 110 || X == 111) && (Y == 20 || Y == 21)) {
						color(hConsole, 5);
						cout << char(219);
					}
					else
						if (X == 110 && Y == 23) {
							cout << "5";
						}
						else
							if ((X == 104 || X == 105 || X == 106) && (Y == 20 || Y == 21)) {
								color(hConsole, 4);
								cout << char(219);
							}
							else
								if (X == 105 && Y == 23) {
									cout << "4";
								}
								else
									if ((X == 99 || X == 100 || X == 101) && (Y == 20 || Y == 21)) {
										color(hConsole, 3);
										cout << char(219);
									}
									else
										if (X == 100 && Y == 23) {
											cout << "3";
										}
										else
											if ((X == 94 || X == 95 || X == 96) && (Y == 20 || Y == 21)) {
												color(hConsole, 2);
												cout << char(219);
											}
											else
												if (X == 95 && Y == 23) {
													cout << "2";
												}
												else
													if ((X == 89 || X == 90 || X == 91) && (Y == 20 || Y == 21)) {
														color(hConsole, 1);
														cout << char(219);
													}
													else
														if (X == 90 && Y == 23) {
															cout << "1";
														}
														else
															if ((X == 118 || X == 119 || X == 120 || X == 121 || X == 122) && (Y == 44 || Y == 45 || Y == 46)) {
																color(hConsole, 7);
																if ((X == 118) && Y == 44) {
																	cout << " ";
																}
																else
																	if ((X == 118) && Y == 45) {
																		cout << char(220);
																	}
																	else
																		if ((X == 118) && Y == 46) {
																			cout << " ";
																		}
																		else
																			if ((X == 119 || X == 120 || X == 121) && Y == 44) {
																				cout << " ";
																			}
																			else
																				if ((X == 119 || X == 120 || X == 121) && Y == 45) {
																					cout << " ";
																				}
																				else
																					if ((X == 119 || X == 120 || X == 121) && Y == 46) {
																						cout << char(223);
																					}
																					else
																						if (X == 122 && Y == 44) {
																							cout << char(219);
																						}
																						else
																							if (X == 122 && Y == 45) {
																								cout << char(219);
																							}
																							else
																								if (X == 122 && Y == 46) {
																									cout << " ";
																								}
															}
															else
																if ((X == 124 || X == 125 || X == 126 || X == 127 || X == 128) && (Y == 44 || Y == 45 || Y == 46)) {
																	color(hConsole, 7);
																	if (X == 124 && (Y == 44)) {
																		cout << char(220);
																	}
																	else
																		if (X == 124 && Y == 45) {
																			cout << " ";
																		}
																		else
																			if (X == 124 && Y == 46) {
																				cout << char(223);
																			}
																			else
																				if (X == 125 && (Y == 44)) {
																					cout << char(223);
																				}
																				else
																					if (X == 125 && Y == 45) {
																						cout << " ";
																					}
																					else
																						if (X == 125 && Y == 46) {
																							cout << char(223);
																						}
																						else
																							if (X == 126 && (Y == 44)) {
																								cout << char(223);
																							}
																							else
																								if (X == 126 && Y == 45) {
																									cout << char(220);
																								}
																								else
																									if (X == 126 && Y == 46) {
																										cout << char(223);
																									}
																									else
																										if (X == 127 && (Y == 44)) {
																											cout << char(223);
																										}
																										else
																											if (X == 127 && Y == 45) {
																												cout << char(223);
																											}
																											else
																												if (X == 127 && Y == 46) {
																													cout << char(223);
																												}
																												else
																													if (X == 128 && (Y == 44)) {
																														cout << char(220);
																													}
																													else
																														if (X == 128 && Y == 45) {
																															cout << " ";
																														}
																														else
																															if (X == 128 && Y == 46) {
																																cout << char(223);
																															}
																}
																else
																	cout << " ";
		}
		else
			cout << " ";


}

// ===== Glifos ASCII 5x3 (letras del nombre) =====
static const unsigned char GLYPH[26][3][5] = {
	/* A */ { { 220, 223, 223, 223, 220 }, { 219, 223, 223, 223, 219 }, { 223, 32, 32, 32, 223 } },
	/* B */ { { 219, 223, 223, 223, 220 }, { 219, 223, 223, 223, 220 }, { 223, 223, 223, 223, 32 } },
	/* C */ { { 220, 223, 223, 223, 220 }, { 219, 32, 32, 32, 220 }, { 32, 223, 223, 223, 32 } },
	/* D */ { { 219, 223, 223, 223, 220 }, { 219, 32, 32, 32, 219 }, { 223, 223, 223, 223, 32 } },
	/* E */ { { 219, 223, 223, 223, 223 }, { 219, 223, 223, 223, 32 }, { 223, 223, 223, 223, 223 } },
	/* F */ { { 219, 223, 223, 223, 223 }, { 219, 223, 223, 223, 32 }, { 223, 32, 32, 32, 32 } },
	/* G */ { { 220, 223, 223, 223, 32 }, { 219, 32, 32, 223, 219 }, { 32, 223, 223, 223, 32 } },
	/* H */ { { 219, 32, 32, 32, 219 }, { 219, 223, 223, 223, 219 }, { 223, 32, 32, 32, 223 } },
	/* I */ { { 223, 223, 219, 223, 223 }, { 32, 32, 219, 32, 32 }, { 223, 223, 223, 223, 223 } },
	/* J */ { { 32, 32, 32, 32, 219 }, { 220, 32, 32, 32, 219 }, { 32, 223, 223, 223, 32 } },
	/* K */ { { 219, 32, 220, 223, 223 }, { 219, 223, 220, 32, 32 }, { 223, 32, 32, 223, 223 } },
	/* L */ { { 219, 32, 32, 32, 32 }, { 219, 32, 32, 32, 32 }, { 223, 223, 223, 223, 223 } },
	/* M */ { { 219, 220, 32, 220, 219 }, { 219, 32, 223, 32, 219 }, { 223, 32, 32, 32, 223 } },
	/* N */ { { 219, 220, 32, 32, 219 }, { 219, 32, 223, 220, 219 }, { 223, 32, 32, 32, 223 } },
	/* O */ { { 220, 223, 223, 223, 220 }, { 219, 32, 32, 32, 219 }, { 32, 223, 223, 223, 32 } },
	/* P */ { { 219, 223, 223, 223, 220 }, { 219, 223, 223, 223, 32 }, { 223, 32, 32, 32, 32 } },
	/* Q */ { { 220, 223, 223, 223, 220 }, { 219, 32, 32, 220, 223 }, { 32, 223, 223, 223, 223 } },
	/* R */ { { 219, 223, 223, 223, 220 }, { 219, 223, 223, 219, 223 }, { 223, 32, 32, 32, 223 } },
	/* S */ { { 220, 223, 223, 223, 223 }, { 32, 223, 223, 223, 220 }, { 223, 223, 223, 223, 32 } },
	/* T */ { { 223, 223, 219, 223, 223 }, { 32, 32, 219, 32, 32 }, { 32, 32, 223, 32, 32 } },
	/* U */ { { 219, 32, 32, 32, 219 }, { 219, 32, 32, 32, 219 }, { 32, 223, 223, 223, 32 } },
	/* V */ { { 219, 32, 32, 32, 219 }, { 223, 220, 32, 220, 223 }, { 32, 32, 223, 32, 32 } },
	/* W */ { { 219, 32, 32, 32, 219 }, { 219, 220, 223, 220, 219 }, { 223, 32, 32, 32, 223 } },
	/* X */ { { 223, 220, 32, 220, 223 }, { 32, 220, 223, 220, 32 }, { 223, 32, 32, 32, 223 } },
	/* Y */ { { 219, 32, 32, 32, 219 }, { 32, 223, 219, 223, 32 }, { 32, 32, 223, 32, 32 } },
	/* Z */ { { 223, 223, 223, 219, 223 }, { 32, 220, 223, 32, 32 }, { 223, 223, 223, 223, 223 } }
};

static bool letraEnBBox(int baseX, int X, int Y) {
	return (Y >= 12 && Y <= 14 && X >= baseX && X <= baseX + 4);
}

static void dibujarGlifoEn(char ch, int baseX, int X, int Y) {
	color(hConsole, 15);
	unsigned char c = 32;
	if (ch >= 'A' && ch <= 'Z') c = GLYPH[ch - 'A'][Y - 12][X - baseX];
	if (c == 32) cout << " ";
	else cout << char(c);
}

static void dibujarDigrama(char a, char b, short cont, int X, int Y) {
	int L = 103 - (cont + 1);
	int R = 103 + (cont + 1);
	if (letraEnBBox(L, X, Y)) dibujarGlifoEn(a, L, X, Y);
	else if (letraEnBBox(R, X, Y)) dibujarGlifoEn(b, R, X, Y);
	else cout << " ";
}

static void dibujarTrigrama(char a, char b, char c, short cont, int X, int Y) {
	int L = 103 - (cont * 2);
	int M = 103;
	int R = 103 + (cont * 2);
	if (letraEnBBox(L, X, Y)) dibujarGlifoEn(a, L, X, Y);
	else if (letraEnBBox(M, X, Y)) dibujarGlifoEn(b, M, X, Y);
	else if (letraEnBBox(R, X, Y)) dibujarGlifoEn(c, R, X, Y);
	else cout << " ";
}


static bool glifoEnBBoxY(int baseX, int baseY, int X, int Y) {
	return (Y >= baseY && Y <= baseY + 2 && X >= baseX && X <= baseX + 4);
}

static void dibujarGlifoColor(char ch, int baseX, int baseY, int X, int Y, int colorAttr) {
	color(hConsole, colorAttr);
	unsigned char c = 32;
	if (ch >= 'A' && ch <= 'Z') c = GLYPH[ch - 'A'][Y - baseY][X - baseX];
	if (c == 32) cout << " ";
	else cout << char(c);
}

static void dibujarSprite5x3(const unsigned char sp[3][5], int baseX, int baseY, int X, int Y, int colorAttr) {
	color(hConsole, colorAttr);
	unsigned char c = sp[Y - baseY][X - baseX];
	if (c == 32) cout << " ";
	else cout << char(c);
}

// Indicadores de jugador en pantalla de nombre (no son letras A-Z)
static const unsigned char SPR_P1[3][5] = {
	{ 32, 223, 219, 32, 32 },
	{ 32, 32, 219, 32, 32 },
	{ 32, 32, 223, 32, 32 }
};
static const unsigned char SPR_P2[3][5] = {
	{ 220, 223, 223, 223, 220 },
	{ 32, 32, 220, 223, 32 },
	{ 223, 223, 223, 223, 223 }
};

static const int TITULO_X[9] = { 76, 82, 88, 94, 100, 106, 112, 118, 130 };
static const char TITULO_CH[9] = { 'N', 'O', 'M', 'B', 'R', 'A', 'T', 'E', 'J' };

static const char* const WORD_CH[3] = { "BORRAR", "ESPACIO", "LISTO" };
static const int WORD_X0[3] = { 42, 88, 141 };
static const int WORD_LEN[3] = { 6, 7, 5 };
static const int WORD_CI[3] = { 26, 27, 28 };

static bool dibujarTecladoNombre(int X, int Y, int* ColorL) {
	int i, k, bx;
	// Titulo "NOMBRATE J" (Y=7..9), color aleatorio por celda
	for (i = 0; i < 9; ++i) {
		if (glifoEnBBoxY(TITULO_X[i], 7, X, Y)) {
			dibujarGlifoColor(TITULO_CH[i], TITULO_X[i], 7, X, Y, 1 + rand() % 15);
			return true;
		}
	}
	// Teclas A-Z (mismas posiciones que NK_*)
	for (i = 0; i < 26; ++i) {
		if (glifoEnBBoxY(NK_X[i], NK_Y[i], X, Y)) {
			dibujarGlifoColor(NK_CH[i], NK_X[i], NK_Y[i], X, Y, 7 + ColorL[i]);
			return true;
		}
	}
	// BORRAR / ESPACIO / LISTO (varias letras de 5px con hueco de 1)
	for (i = 0; i < 3; ++i) {
		for (k = 0; k < WORD_LEN[i]; ++k) {
			bx = WORD_X0[i] + k * 6;
			if (glifoEnBBoxY(bx, 42, X, Y)) {
				dibujarGlifoColor(WORD_CH[i][k], bx, 42, X, Y, 7 + ColorL[WORD_CI[i]]);
				return true;
			}
		}
	}
	return false;
}

static void dibujarNombreIngresado(int& X, int& Y, short& cont, char* Nombre) {
	if (cont == 1) {
		if (Nombre[cont - 1] >= 'A' && Nombre[cont - 1] <= 'Z') {
			if (letraEnBBox(103, X, Y)) dibujarGlifoEn(Nombre[cont - 1], 103, X, Y);
			else cout << " ";
		}
		else cout << " ";
	}
	else if (cont == 2) {
		dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
	}
	else if (cont == 3) {
		if (Nombre[cont - 3] == 'A' || Nombre[cont - 3] == 'B' || Nombre[cont - 3] == 'C') {
			dibujarTrigrama(Nombre[cont - 3], Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
		}
		else if (Nombre[cont - 3] == 'L' && (Nombre[cont - 2] == 'E' || Nombre[cont - 2] == 'V' || Nombre[cont - 2] == 'W' || Nombre[cont - 2] == 'X')) {
			LetrA(X, Y, cont, Nombre);
		}
		else {
			dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
		}
	}
	else if (cont == 4) {
		if (Nombre[cont - 4] == 'J' && (Nombre[cont - 3] == 'U' || Nombre[cont - 2] == 'A' || Nombre[cont - 1] == 'N')) {
			LetrO(X, Y, cont, Nombre);
		}
	}
	else cout << " ";
}


// ===== Mapa: fichas, agujeros y marco =====
static const unsigned char AGUJERO[5][10] = {
	{ 32, 32, 220, 223, 223, 223, 223, 220, 32, 32 },
	{ 220, 223, 32, 32, 32, 32, 32, 32, 223, 220 },
	{ 219, 32, 32, 32, 32, 32, 32, 32, 32, 219 },
	{ 223, 220, 32, 32, 32, 32, 32, 32, 220, 223 },
	{ 32, 32, 223, 220, 220, 220, 220, 223, 32, 32 }
};

static bool dibujarFichaMapa(int X, int Y, short* ColorF) {
	for (int i = 0; i < 24; ++i) {
		int bx = BOARD_X[i], by = BOARD_Y[i];
		if (X < bx || X > bx + 3 || Y < by || Y > by + 2) continue;
		int dx = X - bx, dy = Y - by;
		color(hConsole, ColorF[i]);
		if ((dx == 1 || dx == 2) && dy == 1) {
			color(hConsole, (ColorF[i] * 3) - 1);
			cout << char(219);
		}
		else if ((dx == 0 || dx == 3) && dy == 1) {
			cout << char(219);
		}
		else if ((dx == 1 || dx == 2) && dy == 0) {
			cout << char(220);
		}
		else if ((dx == 1 || dx == 2) && dy == 2) {
			cout << char(223);
		}
		else {
			cout << " ";
		}
		return true;
	}
	return false;
}

static bool dibujarAgujeroMapa(int X, int Y, short* ColorA) {
	for (int i = 0; i < 24; ++i) {
		int x0 = BOARD_X[i] - 3, y0 = BOARD_Y[i] - 1;
		if (X < x0 || X > x0 + 9 || Y < y0 || Y > y0 + 4) continue;
		color(hConsole, 8 + ColorA[i]);
		unsigned char ch = AGUJERO[Y - y0][X - x0];
		if (ch == 32) cout << " ";
		else cout << char(ch);
		return true;
	}
	return false;
}

static bool dibujarLineasTablero(int X, int Y) {
	color(hConsole, 7);
	if ((X == 66 || X == 67 || X == 68 || X == 69 || X == 70 || X == 71 || X == 72 || X == 73 || X == 74 || X == 75 || X == 76 || X == 77 || X == 78 || X == 79 || X == 80 || X == 81 || X == 82 || X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130 || X == 131 || X == 132 || X == 133 || X == 134 || X == 135 || X == 136 || X == 137 || X == 138 || X == 139 || X == 140 || X == 141 || X == 142 || X == 143 || X == 144) && (Y == 5 || Y == 39)) {
		cout << char(219);
		return true;
	}
	if ((X == 150 || X == 149 || X == 60 || X == 61) && (Y == 8 || Y == 9 || Y == 10 || Y == 11 || Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32 || Y == 33 || Y == 34 || Y == 35 || Y == 36)) {
		cout << char(219);
		return true;
	}
	if ((X == 75 || X == 76 || X == 134 || X == 135) && (Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30)) {
		cout << char(219);
		return true;
	}
	if ((X == 81 || X == 82 || X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129) && (Y == 11 || Y == 33)) {
		cout << char(219);
		return true;
	}
	if ((X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114) && (Y == 17 || Y == 27)) {
		cout << char(219);
		return true;
	}
	if ((X == 104 || X == 105) && (Y == 8 || Y == 14 || Y == 30 || Y == 36)) {
		cout << char(219);
		return true;
	}
	if ((X == 66 || X == 67 || X == 68 || X == 69 || X == 70 || X == 81 || X == 82 || X == 83 || X == 84 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 140 || X == 141 || X == 142 || X == 143 || X == 144) && Y == 22) {
		cout << char(219);
		return true;
	}
	return false;
}

static const int TURNO_X[5] = { 82, 88, 94, 100, 106 };
static const char TURNO_CH[5] = { 'T', 'U', 'R', 'N', 'O' };

static bool dibujarTurnoLabel(int X, int Y) {
	for (int i = 0; i < 5; ++i) {
		if (glifoEnBBoxY(TURNO_X[i], 44, X, Y)) {
			dibujarGlifoColor(TURNO_CH[i], TURNO_X[i], 44, X, Y, 15);
			return true;
		}
	}
	return false;
}

static bool dibujarTurnoJugador(int X, int Y, short contL, short* ColorFJ) {
	if (contL == 0) {
		if (glifoEnBBoxY(118, 44, X, Y)) {
			dibujarGlifoColor('J', 118, 44, X, Y, ColorFJ[0]);
			return true;
		}
		if (glifoEnBBoxY(124, 44, X, Y)) {
			dibujarSprite5x3(SPR_P1, 124, 44, X, Y, ColorFJ[0]);
			return true;
		}
		cout << " ";
		return true;
	}
	if (contL == 1) {
		if (glifoEnBBoxY(118, 44, X, Y)) {
			dibujarGlifoColor('J', 118, 44, X, Y, ColorFJ[1]);
			return true;
		}
		if (glifoEnBBoxY(124, 44, X, Y)) {
			dibujarSprite5x3(SPR_P2, 124, 44, X, Y, ColorFJ[1]);
			return true;
		}
		cout << " ";
		return true;
	}
	cout << " ";
	return true;
}

void Mapa(int& X, int& Y, short* turno, short& contL, short* ColorF, short* ColorFJ, bool Molino, short* ColorA) {
	(void)turno;
	(void)Molino;

	color(hConsole, 7);

	if (dibujarFichaMapa(X, Y, ColorF)) return;
	if (dibujarAgujeroMapa(X, Y, ColorA)) return;
	if (dibujarLineasTablero(X, Y)) return;
	if (dibujarTurnoLabel(X, Y)) return;
	dibujarTurnoJugador(X, Y, contL, ColorFJ);
}

void Dibuja_Torre_Izquierda(int& X, int& Y, short* Color) {
	if (X == 4 || X == 5 || X == 6 || X == 7 || X == 8 || X == 9 || X == 10 || X == 11 || X == 12 || X == 13 || X == 14 || X == 15 || X == 16 || X == 17 || X == 18 || X == 19 || X == 20 || X == 21 || X == 22) {  //  Torre Izquierda y Limite de la torre Izquierda ( X de 4 a 22 ) 
		if (Y == 2 || Y == 3 || Y == 4 || Y == 5 || Y == 6 || Y == 7) {  //  Parte de Arriba de la torre Izquierda y Limite de la Parte de Arriba de la torre Izquierda ( Y de 2 a 7 )
			if (Y == 5 && (X == 21 || X == 22) || Y == 6 && (X == 5 || X == 6 || X == 9 || X == 10 || X == 13 || X == 14 || X == 17 || X == 18 || X == 22 || X == 21 || X == 20) || Y == 7 && (X == 22 || X == 21 || X == 20)) {  //  Ventanas y vacios en la Parte de Arriba de la torre Izquierda
				cout << " ";
			}  //  Fin Ventanas y vacios en la Parte de Arriba de la torre Izquierda
			else  //  Forma de la Parte de Arriba de la torre Izquierda
				if (Y == 2 || Y == 6) {
					if (X == 4 || X == 5) {
						color(hConsole, 6);
						cout << char(220);
					}
					else
						if ((X == 5 || X == 6) && Y == 2) {
							color(hConsole, 6);
							cout << char(220);
						}
						else {
							color(hConsole, 14);
							cout << char(220);
						}
				}
				else
					if (Y == 3 || Y == 7 || Y == 5) {
						if (Y == 5 && X == 20) {
							color(hConsole, 14);
							cout << char(223);
						}
						else
							if (X == 4 || X == 5) {
								color(hConsole, 6);
								cout << char(219);
							}
							else {
								color(hConsole, 14);
								cout << char(219);
							}
					}
					else
						if (Y == 4) {
							if (X == 4 || X == 5) {
								color(hConsole, 6);
								cout << char(223);
							}
							else {
								color(hConsole, 14);
								cout << char(223);
							}
						}
		}  //  Fin Forma Parte de Arriba de la torre Izquierda
		else  //  Paredes de la torre Izquierda
			if (Y == 8 || Y == 9 || Y == 10 || Y == 11 || Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32 || Y == 33 || Y == 34 || Y == 35 || Y == 36 || Y == 37 || Y == 38 || Y == 39 || Y == 40 || Y == 41 || Y == 42 || Y == 43 || Y == 44 || Y == 45) {  //  Limite de Paredes de la torre Izquierda ( Y de 8 a 45 )
				if (X == 22) {  //  Paredes que no se usan, Parte de Abajo de la torre Izquierda y Parte del Cubito
					if (Y == 45) {
						color(hConsole, 14);
						cout << char(219);
					}
					else
						if (Y == 32) {
							color(hConsole, 14 - Color[4]);
							cout << char(219);
						}
						else
							if (Y == 40) {
								color(hConsole, 14 - Color[4]);
								cout << char(219);
							}
							else
								cout << " ";
				}
				else
					if (X == 21) {
						if (Y == 45) {
							color(hConsole, 14);
							cout << char(219);
						}
						else
							if (Y == 32) {
								color(hConsole, 14 - Color[4]);
								cout << char(219);
							}
							else
								if (Y == 40) {
									color(hConsole, 14 - Color[4]);
									cout << char(219);
								}
								else
									if (Y == 39 || Y == 38 || Y == 37 || Y == 36 || Y == 35 || Y == 34 || Y == 33 || Y == 32) {
										color(hConsole, 14 - Color[4]);
										cout << char(219);
									}
									else
										cout << " ";
					}
					else//  Fin Paredes que no se usan, Parte de Abajo de la torre Izquierda y Parte del Cubito
						if (X == 20) {  //  Detalles Pared Izquierda
							if (Y == 45) {
								color(hConsole, 14);
								cout << char(219);
							}
							else
								if (Y == 10 || Y == 28) {
									color(hConsole, 14);
									cout << char(187);
								}
								else
									if (Y == 11 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 39 || Y == 38 || Y == 37 || Y == 36 || Y == 34 || Y == 33 || Y == 32 || Y == 31 || Y == 29) {
										color(hConsole, 14);
										cout << char(186);
									}
									else
										if (Y == 12 || Y == 30) {
											color(hConsole, 14);
											cout << char(185);
										}
										else
											if (Y == 17) {
												color(hConsole, 14);
												cout << char(221);
											}
											else
												if (Y == 24 || Y == 42) {
													color(hConsole, 14);
													cout << char(188);
												}
												else
													if (Y == 23 || Y == 41) {
														color(hConsole, 14);
														cout << char(220);
													}
													else
														if (Y == 35) {
															color(hConsole, 14);
															cout << char(204);
														}
														else
															if (Y == 40) {
																color(hConsole, 14);
																cout << char(202);
															}
															else {
																cout << " ";
															}
						}
						else  //  Fin Detalles Pared Izquierda
							if (X == 15) {  //  Ladrillos Pared Izquierda
								if (Y == 8) {
									color(hConsole, 12);
									cout << char(223);
								}
								else
									if (Y == 11 || Y == 15 || Y == 19 || Y == 23 || Y == 27 || Y == 31 || Y == 35 || Y == 39 || Y == 43 || Y == 45) {
										color(hConsole, 12);
										cout << char(219);
									}
									else
										if (Y == 12 || Y == 16 || Y == 20 || Y == 24 || Y == 28 || Y == 32 || Y == 36 || Y == 40 || Y == 44) {
											color(hConsole, 12);
											cout << char(223);
										}
										else {
											cout << " ";
										}
							}
							else
								if (X == 7) {
									if (Y == 8) {
										color(hConsole, 12);
										cout << char(223);
									}
									else
										if (Y == 9 || Y == 13 || Y == 17 || Y == 21 || Y == 25 || Y == 29 || Y == 33 || Y == 37 || Y == 41 || Y == 45) {
											color(hConsole, 12);
											cout << char(219);
										}
										else
											if (Y == 10 || Y == 14 || Y == 18 || Y == 22 || Y == 26 || Y == 30 || Y == 34 || Y == 38 || Y == 42) {
												color(hConsole, 12);
												cout << char(223);
											}
											else {
												cout << " ";
											}
								}
								else  // Fin Ladrillos Pared Izquierda
									if (Y == 8 || Y == 10 || Y == 12 || Y == 14 || Y == 16 || Y == 18 || Y == 20 || Y == 22 || Y == 24 || Y == 26 || Y == 28 || Y == 30 || Y == 32 || Y == 34 || Y == 36 || Y == 38 || Y == 40 || Y == 42 || Y == 44) {  //  Dibuja y pinta Pared Izquierda ( Color Rojo )
										if (X == 6 || X == 7 || X == 8 || X == 9 || X == 10 || X == 11 || X == 12 || X == 13 || X == 14 || X == 15 || X == 16 || X == 17 || X == 18 || X == 19 || X == 20 || X == 21 || X == 22) {
											color(hConsole, 12);
											cout << char(223);
										}
										else {
											color(hConsole, 4);
											cout << char(223);
										}
									}
									else {
										if (X == 6 || X == 7 || X == 8 || X == 9 || X == 10 || X == 11 || X == 12 || X == 13 || X == 14 || X == 15 || X == 16 || X == 17 || X == 18 || X == 19 || X == 20 || X == 21 || X == 22) {
											color(hConsole, 12);
											cout << char(219);
										}
										else {
											color(hConsole, 4);
											cout << char(219);
										}
									}  //  Fin Dibuja y pinta Pared Izquierda ( Color Rojo )
			}  //  Fin Paredes de la torre Izquierda
			else  //  Parte de Abajo de la torre Izquierda
				if (Y == 48 || Y == 47 || Y == 46) {  //  Limite de Parte de Arriba de la torre Izquierda ( Y de 48 a 46 ) 
					if ((X == 22 || X == 21) && (Y == 47 || Y == 48) || X == 20 && Y == 48) {
						cout << " ";
					}
					else
						if (Y == 48) {  //  Forma de la Parte de Abajo de la torre Izquierda
							if (X == 4 || X == 5) {
								color(hConsole, 6);
								cout << char(238);
							}
							else {
								color(hConsole, 14);
								cout << char(238);
							}
						}
						else
							if (Y == 47) {
								if (X == 4 || X == 5) {
									color(hConsole, 6);
									cout << char(220);
								}
								else
									if (X == 20) {
										color(hConsole, 14);
										cout << char(205);
									}
									else {
										color(hConsole, 14);
										cout << char(220);
									}
							}
							else
								if (Y == 46) {
									if (X == 4 || X == 5) {
										color(hConsole, 6);
										cout << char(219);
									}
									else {
										color(hConsole, 14);
										cout << char(219);
									}
								}
				}  //  Fin Parte de Abajo de la torre Izquierda
	}  //  Fin Torre Izquierda
}

void Dibuja_Torre_Derecha(int& X, int& Y) {
	//  Torre Derecha
	if (X == 207 || X == 206 || X == 205 || X == 204 || X == 203 || X == 202 || X == 201 || X == 200 || X == 199 || X == 198 || X == 197 || X == 196 || X == 195 || X == 194 || X == 193 || X == 192 || X == 191 || X == 190 || X == 189) {//Limite de la torre Derecha ( X de 189 a 207 )
		if (Y == 2 || Y == 3 || Y == 4 || Y == 5 || Y == 6 || Y == 7) { // // Parte de Arriba de la torre Derecha y Limite de Parte de Arriba de la torre Derecha ( Y de 2 a 7 )
			if (Y == 7 && (X == 189 || X == 190 || X == 191) || Y == 6 && (X == 189 || X == 190 || X == 191 || X == 206 || X == 205 || X == 202 || X == 201 || X == 198 || X == 197 || X == 194 || X == 193) || Y == 5 && (X == 189 || X == 190)) {  //  Ventanas y vacios en la Parte de Arriba de la torre Derecha
				cout << " ";
			}
			else  //  Fin Ventanas y vacios en la Parte de Arriba de la torre Derecha
				if (Y == 5 && X == 191) {  //  Forma de la Parte de Arriba de la torre Derecha
					color(hConsole, 6);
					cout << char(223);
				}
				else
					if (Y == 2 || Y == 6) {
						if (X == 189 || X == 190 || X == 191 || X == 192 || X == 193) {
							color(hConsole, 6);
							cout << char(220);
						}
						else {
							color(hConsole, 14);
							cout << char(220);
						}

					}
					else
						if (Y == 3 || Y == 7 || Y == 5) {
							if (X == 189 || X == 190 || X == 191 || X == 192 || X == 193) {
								color(hConsole, 6);
								cout << char(219);
							}
							else {
								color(hConsole, 14);
								cout << char(219);
							}

						}
						else
							if (Y == 4) {
								if (X == 189 || X == 190 || X == 191 || X == 192 || X == 193) {
									color(hConsole, 6);
									cout << char(223);
								}
								else {
									color(hConsole, 14);
									cout << char(223);
								}
							}
		}  // Fin Parte de Arriba de la torre Derecha
		else  // Paredes de la torre Derecha
			if (Y == 8 || Y == 9 || Y == 10 || Y == 11 || Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32 || Y == 33 || Y == 34 || Y == 35 || Y == 36 || Y == 37 || Y == 38 || Y == 39 || Y == 40 || Y == 41 || Y == 42 || Y == 43 || Y == 44 || Y == 45) {
				if (X == 189) {  //  Paredes que no se usan, Parte ventilador y Parte de abajo de la Torre Derecha
					if (Y == 38 || Y == 37) {
						cout << " ";
					}
					else
						if (Y == 45 || Y == 40 || Y == 39 || Y == 36 || Y == 35 || Y == 34 || Y == 33 || Y == 32) {
							color(hConsole, 6);
							cout << char(219);
						}
						else {
							cout << " ";
						}
				}
				else
					if (X == 190) {
						if (Y == 45 || Y == 40 || Y == 39 || Y == 38 || Y == 37 || Y == 36 || Y == 35 || Y == 34 || Y == 33 || Y == 32) {
							color(hConsole, 6);
							cout << char(219);
						}
						else
							cout << " ";
					}
					else  //  Fin Paredes que no se usan, Parte ventilador y Parte de abajo de la Torre Derecha
						if (X == 191) { //  Detalles Pared Derecha
							if (Y == 40) {
								color(hConsole, 6);
								cout << char(202);
							}
							else
								if (Y == 35) {
									color(hConsole, 6);
									cout << char(185);
								}
								else
									if (Y == 39 || Y == 38 || Y == 37 || Y == 36) {
										color(hConsole, 6);
										cout << char(186);
									}
									else
										if (Y == 45) {
											color(hConsole, 6);
											cout << char(219);
										}
										else
											if (Y == 10 || Y == 28) {
												color(hConsole, 6);
												cout << char(201);
											}
											else
												if (Y == 11 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 40 || Y == 39 || Y == 38 || Y == 37 || Y == 36 || Y == 34 || Y == 33 || Y == 32 || Y == 31 || Y == 29) {
													color(hConsole, 6);
													cout << char(186);
												}
												else
													if (Y == 12 || Y == 30) {
														color(hConsole, 6);
														cout << char(204);
													}
													else
														if (Y == 17 || Y == 35) {
															color(hConsole, 6);
															cout << char(221);
														}
														else
															if (Y == 24 || Y == 42) {
																color(hConsole, 6);
																cout << char(200);
															}
															else
																if (Y == 23 || Y == 41) {
																	color(hConsole, 6);
																	cout << char(220);
																}
																else {
																	cout << " ";
																}
						}
						else  //  Fin Detalles Pared Derecha
							if (X == 196) {  //  Ladrillos Pared Derecha
								if (Y == 8 || Y == 12 || Y == 16 || Y == 20 || Y == 24 || Y == 28 || Y == 32 || Y == 36 || Y == 40 || Y == 44) {
									color(hConsole, 13);
									cout << char(223);
								}
								else
									if (Y == 11 || Y == 15 || Y == 19 || Y == 23 || Y == 27 || Y == 31 || Y == 35 || Y == 39 || Y == 43 || Y == 45) {
										color(hConsole, 13);
										cout << char(219);
									}
									else {
										cout << " ";
									}
							}
							else
								if (X == 204) {
									if (Y == 8 || Y == 10 || Y == 14 || Y == 18 || Y == 22 || Y == 26 || Y == 30 || Y == 34 || Y == 38 || Y == 42) {
										color(hConsole, 13);
										cout << char(223);
									}
									else
										if (Y == 9 || Y == 13 || Y == 17 || Y == 21 || Y == 25 || Y == 29 || Y == 33 || Y == 37 || Y == 41 || Y == 45) {
											color(hConsole, 13);
											cout << char(219);
										}
										else {
											cout << " ";
										}
								}  //  Fin Ladrillos Pared Derecha
								else  //  Dibuja y pinta Pared Derecha ( Color Morado )
									if (Y == 8 || Y == 10 || Y == 12 || Y == 14 || Y == 16 || Y == 18 || Y == 20 || Y == 22 || Y == 24 || Y == 26 || Y == 28 || Y == 30 || Y == 32 || Y == 34 || Y == 36 || Y == 38 || Y == 40 || Y == 42 || Y == 44) {
										if (X == 207 || X == 206 || X == 205 || X == 204 || X == 203 || X == 202 || X == 201 || X == 200 || X == 199 || X == 198 || X == 197 || X == 196 || X == 195 || X == 194) {
											color(hConsole, 13);
											cout << char(223);
										}
										else {
											color(hConsole, 5);
											cout << char(223);
										}
									}
									else {
										if (X == 207 || X == 206 || X == 205 || X == 204 || X == 203 || X == 202 || X == 201 || X == 200 || X == 199 || X == 198 || X == 197 || X == 196 || X == 195 || X == 194) {
											color(hConsole, 13);
											cout << char(219);
										}
										else {
											color(hConsole, 5);
											cout << char(219);
										}
									}
				//  Fin Dibuja y pinta Pared Derecha ( Color Morado )							
			}  //  Fin de Paredes de la torre Derecha
			else  //  Parte de Abajo de la torre Derecha
				if (Y == 48 || Y == 47 || Y == 46) {// Limite de Parte de Abajo de la torre Derecha ( Y de 46 a 48 )
					if ((X == 189 || X == 190) && (Y == 47 || Y == 48)) {
						cout << " ";
					}
					else
						if (X == 191 && Y == 48) {
							cout << " ";
						}
						else
							if (X == 191 && Y == 47) {
								color(hConsole, 6);
								cout << char(205);
							}
							else
								if (Y == 48) {
									if (X == 189 || X == 190 || X == 191 || X == 192 || X == 193) {
										color(hConsole, 6);
										cout << char(238);
									}
									else {
										color(hConsole, 14);
										cout << char(238);
									}
								}
								else
									if (Y == 47) {
										if (X == 189 || X == 190 || X == 191 || X == 192 || X == 193) {
											color(hConsole, 6);
											cout << char(220);
										}
										else {
											color(hConsole, 14);
											cout << char(220);
										}
									}
									else
										if (Y == 46) {
											if (X == 189 || X == 190 || X == 191 || X == 192 || X == 193) {
												color(hConsole, 6);
												cout << char(219);
											}
											else {
												color(hConsole, 14);
												cout << char(219);
											}
										}
				}  //  Fin Parte de Abajo de la torre Derecha
	}  //  Fin Torre Derecha
}

void Dibuja_Cubito(int& X, int& Y) {
	if (Y == 40 && (X == 23 || X == 24 || X == 25 || X == 26 || X == 27 || X == 28 || X == 29 || X == 30 || X == 31 || X == 32 || X == 33 || X == 34 || X == 35 || X == 36 || X == 37 || X == 38)) {
		cout << char(219);
	}
	else
		if (Y == 32 && (X == 23 || X == 24 || X == 25 || X == 26 || X == 27 || X == 28 || X == 29 || X == 30 || X == 31 || X == 32 || X == 33 || X == 34 || X == 35 || X == 36 || X == 37 || X == 38)) {
			cout << char(219);
		}
		else
			if ((Y == 33 || Y == 34 || Y == 35 || Y == 36 || Y == 37 || Y == 38 || Y == 39) && X == 38) {
				cout << char(219);
			}
			else
				if (Y == 33 && X == 34) {
					cout << char(220);
				}
				else
					if (Y == 34 && X == 33) {
						cout << char(223);
					}
					else
						if (Y == 34 && X == 32) {
							cout << char(220);
						}
						else
							if (Y == 35 && X == 31) {
								cout << char(223);
							}
							else
								if (Y == 35 && X == 30) {
									cout << char(220);
								}
								else
									if (Y == 36 && X == 29) {
										cout << char(223);
									}
									else
										if (Y == 36 && X == 28) {
											cout << char(220);
										}
										else
											if (Y == 34 && X == 36) {
												cout << char(220);
											}
											else
												if (Y == 35 && X == 35) {
													cout << char(223);
												}
												else
													if (Y == 35 && X == 34) {
														cout << char(220);
													}
													else
														if (Y == 36 && X == 33) {
															cout << char(223);
														}
														else
															cout << " ";
}

void Dibuja_Ventilador(int& X, int& Y) {
	if (X == 188 || X == 187 || X == 186 || X == 185 || X == 184 || X == 183 || X == 182 || X == 181 || X == 180 || X == 179 || X == 178 || X == 177 || X == 176 || X == 175 || X == 174 || X == 173 || X == 172 || X == 171) {  //  Limite de Ventilador ( X de 171 a 188 )																			
		if (Y == 39 && (X == 181 || X == 184 || X == 185 || X == 186 || X == 187) || Y == 38 && (X == 184 || X == 182 || X == 181 || X == 180 || X == 179) || Y == 37 && (X == 188 || X == 187 || X == 186 || X == 184 || X == 179) || Y == 36 && (X == 187 || X == 184 || X == 183 || X == 182 || X == 181)) {  //  Dibuja Descarga de aire exterior ( Muy caliente )
			cout << " ";
		}
		else
			if (Y == 40 && (X == 182 || X == 183 || X == 184 || X == 185 || X == 186) || Y == 37 && (X == 182 || X == 183) || Y == 38 && X == 183 || Y == 39 && X == 180) {
				color(hConsole, 9);
				cout << char(220);
			}
			else
				if (Y == 35 && (X == 182 || X == 183 || X == 184 || X == 185 || X == 186) || Y == 36 && X == 180 || (Y == 37 || Y == 38) && X == 185) {
					color(hConsole, 9);
					cout << char(223);
				}
				else
					if ((Y == 36) && X == 188) {
						color(hConsole, 6);
						cout << char(223);
					}
					else
						if ((Y == 39) && (X == 182)) {
							color(hConsole, 9);
							cout << char(219);
						}
						else
							if ((Y == 39) && X == 188) {
								color(hConsole, 6);
								cout << char(220);
							}
							else  //  Fin Dibuja Descarga de aire exterior ( Muy caliente )
								if (X == 188 || X == 187) {  //  Dibuja y pinta parte Derecha del Cuadro del Ventilador ( Color amarillo )
									if (Y == 40 || Y == 39 || Y == 38 || Y == 37 || Y == 36 || Y == 35 || Y == 34 || Y == 33 || Y == 32) {
										color(hConsole, 6);
										cout << char(219);
									}
									else {
										cout << " ";
									}
								}
								else  //  Fin Dibuja y pinta parte Derecha del Cuadro del Ventilador ( Color amarillo )
									if (X == 186 || X == 185 || X == 184 || X == 183 || X == 182 || X == 181 || X == 180 || X == 179 || X == 178 || X == 177 || X == 176 || X == 175 || X == 174 || X == 173 || X == 172 || X == 171) {
										if (Y == 32 && (X == 171 || X == 172)) {  //  Cola de Arriba del Ventilador ( Color Blanco )
											color(hConsole, 7);
											cout << char(220);
										}
										else
											if (Y == 33 && (X == 174 || X == 173)) {  //  Cola de Arriba del Ventilador ( Color Gris )
												color(hConsole, 8);
												cout << char(223);
											}
											else  //  Fin Cola de Arriba del Ventilador ( Color Blanco )
												if (Y == 39 && (X == 171 || X == 173 || X == 174)) {  //  Cola de Abajo del Ventilador ( Color Blanco )
													color(hConsole, 7);
													cout << char(223);
												}
												else
													if (Y == 39 && X == 172) {  //  Cola de Abajo del Ventilador ( Color Gris )
														color(hConsole, 8);
														cout << char(220);
													}  // Fin Cola de Abajo del Ventilador ( Color Gris )
													else  //  Fin Cola de Abajo del Ventilador ( Color Blanco )
														if ((Y == 38) && (X == 172 || X == 173 || X == 174 || X == 171) || (Y == 40 || Y == 37 || Y == 36 || Y == 35 || Y == 34 || Y == 32) && (X == 171 || X == 172 || X == 173 || X == 174) || Y == 33 && (X == 171 || X == 172)) {  //  Huecos del Ventilador y partes que no se usan
															cout << " ";
														}
														else  //  Fin Huecos del Ventilador y partes que no se usan
															if ((Y == 33 || Y == 34) && (X == 175 || X == 176 || X == 177 || X == 181 || X == 182 || X == 183 || X == 184 || X == 185 || X == 186)) {  //  Dibuja Forma de Toma de aire interior ( Caliente )
																color(hConsole, 3);
																cout << char(220);
															}
															else  //  Fin Dibuja Forma de Toma de aire interior ( Caliente )
																if ((Y == 35 || Y == 36 || Y == 37 || Y == 38 || Y == 39) && (X == 175 || X == 176 || X == 177)) {//  Dibuja Forma de Impulsi�n de aire interior ( Frio )
																	color(hConsole, 3);
																	cout << char(220);
																}
																else
																	if (Y == 40 && X == 174) {
																		color(hConsole, 9);
																		cout << char(223);
																	}  //  Fin Dibuja Forma Impulsi�n de aire interior ( Frio )
																	else  //  Dibuja y pinta parte Izquierda del Cuadro del Ventilador ( Color Azul )
																		if (Y == 40 || Y == 39 || Y == 38 || Y == 37 || Y == 36 || Y == 35 || Y == 34 || Y == 33 || Y == 32) {
																			color(hConsole, 9);
																			cout << char(219);
																		}
																		else {
																			cout << " ";
																		} //  Fin Dibuja y pinta parte Izquierda del Cuadro del Ventilador ( Color Azul )
									}
	}
	else
		cout << " ";
}

// ===== Titulos (mapas densos; siempre 1 cout por celda) =====
static const unsigned char TITLE_L_CH[17][39] = {
	{ 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 219, 220, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 220, 220, 220, 220, 220, 220 },
	{ 220, 223, 223, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 220, 32, 32, 32, 32, 32, 32, 32, 32, 32, 219, 223, 223, 219, 219, 219 },
	{ 219, 32, 32, 219, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 219, 219, 223, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 219, 32, 32, 219, 219, 219 },
	{ 219, 220, 32, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 32, 32, 32, 223, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 219, 32, 32, 219, 219, 219 },
	{ 32, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 219, 223, 223, 219, 219, 32, 32, 32, 32, 32, 32, 220, 32, 32, 32, 32, 32, 32, 32, 32, 32, 219, 32, 32, 219, 219, 219 },
	{ 32, 32, 32, 32, 223, 223, 223, 223, 223, 223, 32, 32, 219, 32, 32, 219, 219, 220, 220, 220, 220, 220, 220, 219, 219, 220, 32, 32, 32, 32, 32, 32, 32, 219, 32, 32, 219, 219, 219 },
	{ 32, 32, 32, 32, 223, 32, 32, 223, 223, 223, 32, 32, 219, 32, 32, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 223, 32, 32, 32, 32, 32, 32, 219, 32, 32, 219, 219, 219 },
	{ 32, 32, 32, 32, 223, 32, 32, 223, 223, 223, 32, 32, 219, 32, 32, 219, 219, 32, 32, 32, 32, 32, 32, 219, 223, 32, 32, 32, 32, 32, 32, 32, 32, 219, 220, 220, 219, 219, 219 },
	{ 32, 32, 32, 32, 219, 32, 32, 219, 219, 219, 32, 32, 219, 32, 32, 219, 219, 32, 32, 32, 32, 32, 32, 32, 32, 32, 220, 219, 220, 32, 32, 32, 32, 219, 219, 219, 219, 219, 219 },
	{ 32, 32, 32, 32, 219, 32, 32, 219, 219, 219, 32, 32, 219, 32, 32, 219, 219, 32, 32, 32, 32, 32, 32, 32, 220, 219, 219, 219, 219, 219, 220, 32, 32, 219, 219, 219, 219, 219, 219 },
	{ 32, 32, 32, 32, 219, 32, 32, 219, 219, 219, 32, 32, 219, 32, 32, 219, 219, 220, 220, 220, 220, 220, 220, 220, 220, 220, 219, 219, 219, 32, 32, 32, 32, 219, 219, 219, 219, 219, 219 },
	{ 32, 32, 32, 32, 219, 32, 32, 219, 219, 219, 32, 32, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 32, 32, 32, 32, 219, 219, 219, 219, 219, 219 },
	{ 32, 32, 32, 32, 219, 219, 219, 219, 219, 219, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32 },
	{ 32, 32, 32, 32, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 220, 32, 32, 32, 32 },
	{ 32, 32, 32, 32, 32, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 219, 219, 219, 219, 32, 32, 32, 32 },
	{ 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 223, 219, 219, 219, 219, 219, 219, 223, 32, 32 },
	{ 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 223, 219, 219, 223, 32, 32, 32, 32 },
};
static const unsigned char TITLE_L_COL[17][39] = {
	{ 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 10, 10, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 5, 5, 5, 13, 13, 13 },
	{ 2, 2, 2, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 255, 255, 255, 255, 255, 255, 255, 255, 255, 5, 13, 13, 13, 13, 13 },
	{ 2, 255, 255, 10, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 10, 10, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 5, 255, 255, 13, 13, 13 },
	{ 2, 2, 255, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 255, 255, 255, 2, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 5, 255, 255, 13, 13, 13 },
	{ 255, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 10, 10, 10, 10, 255, 255, 255, 255, 255, 255, 2, 255, 255, 255, 255, 255, 255, 255, 255, 255, 5, 255, 255, 13, 13, 13 },
	{ 255, 255, 255, 255, 3, 3, 11, 11, 11, 11, 255, 255, 2, 255, 255, 10, 10, 2, 2, 2, 2, 2, 2, 2, 10, 10, 255, 255, 255, 255, 255, 255, 255, 5, 255, 255, 13, 13, 13 },
	{ 255, 255, 255, 255, 3, 255, 255, 11, 11, 11, 255, 255, 2, 255, 255, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 255, 255, 255, 255, 255, 255, 5, 255, 255, 13, 13, 13 },
	{ 255, 255, 255, 255, 3, 255, 255, 11, 11, 11, 255, 255, 2, 255, 255, 10, 10, 255, 255, 255, 255, 255, 255, 10, 10, 255, 255, 255, 255, 255, 255, 255, 255, 5, 13, 13, 13, 13, 13 },
	{ 255, 255, 255, 255, 3, 255, 255, 11, 11, 11, 255, 255, 2, 255, 255, 10, 10, 255, 255, 255, 255, 255, 255, 255, 255, 255, 2, 10, 10, 255, 255, 255, 255, 5, 13, 13, 13, 13, 13 },
	{ 255, 255, 255, 255, 3, 255, 255, 11, 11, 11, 255, 255, 2, 255, 255, 10, 10, 255, 255, 255, 255, 255, 255, 255, 2, 2, 10, 10, 10, 10, 10, 255, 255, 5, 13, 13, 13, 13, 13 },
	{ 255, 255, 255, 255, 3, 255, 255, 11, 11, 11, 255, 255, 2, 255, 255, 10, 10, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 10, 10, 255, 255, 255, 255, 5, 13, 13, 13, 13, 13 },
	{ 255, 255, 255, 255, 3, 255, 255, 11, 11, 11, 255, 255, 2, 2, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 255, 255, 255, 255, 5, 5, 13, 13, 13, 13 },
	{ 255, 255, 255, 255, 3, 11, 11, 11, 11, 11, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
	{ 255, 255, 255, 255, 3, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 255, 255, 255, 255 },
	{ 255, 255, 255, 255, 255, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 11, 11, 11, 255, 255, 255, 255 },
	{ 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 3, 3, 11, 11, 11, 11, 11, 11, 255, 255 },
	{ 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 3, 11, 11, 11, 255, 255, 255, 255 },
};
void Dibuja_L_Titulo(int& X, int& Y) {
	if (X < 39 || X > 77 || Y < 7 || Y > 23) {
		cout << " ";
		return;
	}
	{
		int dx = X - 39, dy = Y - 7;
		unsigned char col = TITLE_L_COL[dy][dx];
		unsigned char ch = TITLE_L_CH[dy][dx];
		if (col != 255) color(hConsole, col);
		if (ch == 32) cout << " ";
		else cout << char(ch);
	}
}

static const unsigned char TITLE_E_CH[17][36] = {
	{ 220, 220, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 220, 220, 32, 32, 32, 32, 32, 32 },
	{ 219, 219, 219, 220, 220, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 220, 219, 219, 219, 219, 220, 32, 32, 32, 32 },
	{ 219, 219, 219, 219, 219, 219, 220, 220, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 223, 223, 219, 219, 219, 219, 223, 223, 32, 32, 32 },
	{ 32, 32, 223, 219, 219, 219, 219, 219, 219, 220, 220, 32, 32, 32, 32, 32, 32, 32, 32, 220, 220, 220, 220, 220, 32, 32, 32, 219, 219, 219, 219, 32, 32, 32, 32, 32 },
	{ 32, 32, 32, 32, 32, 223, 219, 219, 219, 219, 219, 219, 220, 220, 32, 32, 32, 32, 32, 219, 32, 32, 219, 219, 32, 220, 219, 219, 219, 219, 219, 32, 32, 32, 220, 220 },
	{ 32, 32, 32, 32, 32, 32, 32, 32, 223, 219, 219, 219, 219, 219, 219, 220, 220, 32, 32, 219, 32, 32, 219, 219, 219, 219, 219, 219, 223, 223, 32, 32, 32, 32, 223, 223 },
	{ 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 223, 219, 219, 219, 219, 219, 219, 32, 219, 32, 32, 219, 219, 219, 223, 32, 32, 32, 32, 32, 32, 32, 32, 219, 219 },
	{ 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 223, 219, 219, 219, 32, 219, 219, 219, 219, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 219, 219 },
	{ 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 223, 32, 223, 223, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 219, 219 },
	{ 32, 220, 219, 219, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 219, 219, 220, 32, 32, 32, 219, 219 },
	{ 32, 219, 219, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 220, 223, 32, 220, 32, 219, 219, 32, 32, 32, 219, 219 },
	{ 32, 219, 219, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 223, 32, 32, 223, 32, 32, 219, 219, 32, 32, 32, 219, 219 },
	{ 32, 219, 219, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 219, 219, 32, 32, 32, 219, 219 },
	{ 32, 219, 219, 220, 32, 32, 32, 32, 32, 32, 32, 32, 32, 220, 32, 32, 32, 223, 220, 220, 32, 32, 32, 32, 32, 32, 32, 32, 220, 219, 219, 32, 32, 32, 220, 220 },
	{ 32, 32, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 32, 32, 220, 219, 223, 32 },
	{ 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 223, 219, 219, 219, 220, 220, 220, 220, 220, 220, 220, 220, 220, 220, 220, 220, 220, 219, 223, 32, 32, 32 },
	{ 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32 },
};
static const unsigned char TITLE_E_COL[17][36] = {
	{ 13, 13, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 6, 14, 255, 255, 255, 255, 255, 255 },
	{ 13, 13, 13, 13, 13, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 6, 6, 14, 14, 14, 14, 255, 255, 255, 255 },
	{ 13, 13, 13, 13, 13, 13, 13, 13, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 6, 6, 6, 14, 14, 14, 14, 14, 255, 255, 255 },
	{ 255, 255, 5, 5, 13, 13, 13, 13, 13, 13, 13, 255, 255, 255, 255, 255, 255, 255, 255, 6, 6, 6, 6, 6, 255, 255, 255, 6, 14, 14, 14, 255, 255, 255, 255, 255 },
	{ 255, 255, 255, 255, 255, 5, 5, 13, 13, 13, 13, 13, 13, 13, 255, 255, 255, 255, 255, 6, 255, 255, 14, 14, 255, 6, 6, 14, 14, 14, 14, 255, 255, 255, 2, 2 },
	{ 255, 255, 255, 255, 255, 255, 255, 255, 5, 5, 13, 13, 13, 13, 13, 13, 13, 255, 255, 6, 255, 255, 14, 14, 14, 14, 14, 14, 14, 14, 255, 255, 255, 255, 2, 10 },
	{ 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 5, 5, 13, 13, 13, 13, 13, 255, 6, 255, 255, 14, 14, 14, 14, 255, 255, 255, 255, 255, 255, 255, 255, 2, 10 },
	{ 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 5, 5, 13, 13, 255, 6, 14, 14, 14, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 2, 10 },
	{ 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 5, 255, 6, 6, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 2, 10 },
	{ 255, 2, 2, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 255, 255, 255, 2, 10 },
	{ 255, 2, 10, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 10, 10, 255, 10, 255, 10, 10, 255, 255, 255, 2, 10 },
	{ 255, 2, 10, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 10, 255, 255, 10, 255, 255, 10, 10, 255, 255, 255, 2, 10 },
	{ 255, 2, 10, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 10, 10, 255, 255, 255, 2, 10 },
	{ 255, 2, 10, 10, 255, 255, 255, 255, 255, 255, 255, 255, 255, 10, 255, 255, 255, 10, 10, 10, 255, 255, 255, 255, 255, 255, 255, 255, 10, 10, 10, 255, 255, 255, 2, 10 },
	{ 255, 255, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 255, 255, 10, 10, 10, 255 },
	{ 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 255, 255, 255 },
	{ 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
};
void Dibuja_E_Titulo(int& X, int& Y) {
	if (X < 78 || X > 113 || Y < 7 || Y > 23) {
		cout << " ";
		return;
	}
	{
		int dx = X - 78, dy = Y - 7;
		unsigned char col = TITLE_E_COL[dy][dx];
		unsigned char ch = TITLE_E_CH[dy][dx];
		if (col != 255) color(hConsole, col);
		if (ch == 32) cout << " ";
		else cout << char(ch);
	}
}

static const unsigned char TITLE_I_CH[16][59] = {
	{ 32, 32, 32, 32, 32, 32, 32, 32, 220, 219, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 220, 219, 219, 219, 219, 219, 219, 219, 220, 32, 32, 32 },
	{ 32, 32, 32, 32, 32, 32, 32, 219, 219, 219, 219, 219, 219, 219, 220, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 220, 32, 32, 220, 254, 223, 223, 220, 32, 254, 254, 32, 32, 32, 32, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 32 },
	{ 32, 32, 32, 32, 32, 32, 32, 219, 219, 219, 219, 219, 219, 219, 219, 220, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 223, 223, 32, 220, 254, 220, 32, 223, 219, 220, 32, 32, 32, 219, 219, 219, 219, 223, 32, 32, 32, 254, 223, 219, 219, 219, 219, 219 },
	{ 32, 32, 32, 32, 32, 32, 32, 32, 223, 219, 32, 32, 223, 219, 219, 219, 219, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 223, 254, 220, 220, 32, 32, 32, 219, 219, 219, 219, 220, 32, 32, 32, 32, 220, 219, 219, 219, 219, 219 },
	{ 220, 220, 220, 220, 220, 220, 220, 220, 220, 220, 220, 220, 220, 220, 219, 219, 219, 219, 220, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 220, 220, 220, 220, 220, 220, 220, 220, 32, 32, 32, 32, 32, 32, 32, 32, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 32 },
	{ 219, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 219, 219, 219, 219, 219, 219, 219, 219, 219, 32, 32, 32, 32, 32, 32, 32, 32, 32, 219, 32, 32, 219, 219, 219, 219, 219, 219, 220, 220, 32, 32, 32, 32, 32, 32, 32, 223, 219, 219, 219, 219, 219, 219, 219, 223, 32, 32, 32 },
	{ 219, 220, 220, 220, 220, 220, 220, 220, 220, 220, 220, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 220, 32, 32, 32, 32, 32, 32, 32, 219, 32, 32, 219, 219, 219, 219, 219, 219, 219, 219, 219, 220, 220, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 220, 220, 220, 220, 220, 220 },
	{ 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 220, 220, 220, 220, 220, 220, 220, 220, 220, 220, 219, 220, 32, 32, 32, 219, 32, 32, 219, 219, 219, 32, 32, 223, 219, 219, 219, 219, 219, 219, 220, 220, 32, 32, 32, 32, 32, 32, 32, 220, 32, 32, 220, 220, 220 },
	{ 219, 223, 223, 223, 223, 223, 223, 223, 223, 219, 220, 32, 32, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 220, 32, 219, 32, 32, 219, 219, 219, 32, 32, 32, 32, 32, 223, 219, 219, 219, 219, 219, 219, 220, 220, 32, 32, 32, 32, 220, 32, 32, 220, 220, 220 },
	{ 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 223, 32, 32, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 223, 32, 219, 32, 32, 219, 219, 219, 32, 32, 32, 32, 32, 32, 32, 32, 223, 219, 219, 219, 219, 219, 219, 220, 220, 32, 220, 32, 32, 220, 220, 220 },
	{ 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 219, 219, 219, 32, 32, 32, 32, 32, 32, 32, 32, 219, 223, 32, 32, 32, 219, 220, 220, 219, 219, 219, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 223, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219 },
	{ 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 219, 219, 219, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 219, 219, 219, 219, 219, 219, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 223, 219, 219, 219, 219, 219, 219, 219, 219, 219 },
	{ 219, 32, 219, 32, 219, 32, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 32, 32, 32, 32, 32, 32, 32, 32, 32, 220, 220, 220, 220, 219, 219, 219, 219, 219, 219, 220, 220, 220, 220, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 223, 223, 223, 223, 223, 223, 223 },
	{ 219, 32, 223, 32, 223, 32, 223, 223, 223, 223, 223, 223, 223, 219, 219, 219, 32, 220, 220, 220, 220, 220, 220, 220, 220, 32, 223, 219, 219, 219, 219, 219, 219, 219, 219, 219, 219, 223, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32 },
	{ 219, 32, 220, 32, 220, 32, 220, 220, 220, 220, 220, 220, 220, 219, 219, 223, 32, 219, 220, 220, 220, 220, 220, 219, 219, 32, 32, 32, 223, 219, 219, 219, 219, 219, 219, 223, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32 },
	{ 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 223, 219, 219, 223, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32 },
};
static const unsigned char TITLE_I_COL[16][59] = {
	{ 255, 255, 255, 255, 255, 255, 255, 255, 11, 11, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 11, 11, 11, 11, 11, 11, 11, 11, 11, 255, 255, 255 },
	{ 255, 255, 255, 255, 255, 255, 255, 11, 11, 11, 11, 11, 11, 11, 11, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 4, 255, 255, 4, 4, 4, 4, 4, 255, 4, 4, 255, 255, 255, 255, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 255 },
	{ 255, 255, 255, 255, 255, 255, 255, 3, 11, 11, 11, 11, 11, 11, 11, 11, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 4, 4, 255, 4, 4, 4, 255, 4, 4, 4, 255, 255, 255, 3, 11, 11, 11, 11, 255, 255, 255, 11, 11, 11, 11, 11, 11, 11 },
	{ 255, 255, 255, 255, 255, 255, 255, 255, 3, 3, 255, 255, 3, 3, 11, 11, 11, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 4, 4, 4, 4, 255, 255, 255, 3, 11, 11, 11, 11, 255, 255, 255, 255, 11, 11, 11, 11, 11, 11 },
	{ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 11, 11, 11, 11, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 6, 6, 14, 14, 14, 14, 14, 14, 255, 255, 255, 255, 255, 255, 255, 255, 3, 3, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 255 },
	{ 3, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 255, 255, 255, 255, 255, 255, 255, 255, 255, 6, 255, 255, 14, 14, 14, 14, 14, 14, 14, 14, 255, 255, 255, 255, 255, 255, 255, 3, 3, 11, 11, 11, 11, 11, 11, 11, 255, 255, 255 },
	{ 3, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 255, 255, 255, 255, 255, 255, 255, 6, 255, 255, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 6, 6, 14, 14, 14, 14 },
	{ 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 13, 255, 255, 255, 6, 255, 255, 14, 14, 14, 255, 255, 6, 6, 14, 14, 14, 14, 14, 14, 14, 255, 255, 255, 255, 255, 255, 255, 6, 255, 255, 14, 14, 14 },
	{ 5, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 255, 255, 5, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 255, 6, 255, 255, 14, 14, 14, 255, 255, 255, 255, 255, 6, 6, 14, 14, 14, 14, 14, 14, 14, 255, 255, 255, 255, 6, 255, 255, 14, 14, 14 },
	{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 255, 255, 5, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 255, 6, 255, 255, 14, 14, 14, 255, 255, 255, 255, 255, 255, 255, 255, 6, 6, 14, 14, 14, 14, 14, 14, 14, 255, 6, 255, 255, 14, 14, 14 },
	{ 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 5, 13, 13, 255, 255, 255, 255, 255, 255, 255, 255, 13, 13, 255, 255, 255, 6, 14, 14, 14, 14, 14, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 6, 6, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
	{ 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 5, 13, 13, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 6, 14, 14, 14, 14, 14, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 6, 6, 14, 14, 14, 14, 14, 14, 14, 14 },
	{ 5, 255, 13, 255, 13, 255, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 255, 255, 255, 255, 255, 255, 255, 255, 255, 6, 6, 6, 6, 6, 14, 14, 14, 14, 14, 14, 14, 14, 14, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 14, 14, 14, 14, 14, 14, 14 },
	{ 5, 255, 13, 255, 13, 255, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 255, 5, 13, 13, 13, 13, 13, 13, 13, 255, 6, 6, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
	{ 5, 255, 5, 255, 5, 255, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 255, 5, 5, 5, 5, 5, 13, 13, 13, 255, 255, 255, 6, 6, 14, 14, 14, 14, 14, 14, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
	{ 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 6, 14, 14, 14, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
};
void Dibuja_I_Titulo(int& X, int& Y) {
	if (X < 116 || X > 174 || Y < 8 || Y > 23) {
		cout << " ";
		return;
	}
	{
		int dx = X - 116, dy = Y - 8;
		unsigned char col = TITLE_I_COL[dy][dx];
		unsigned char ch = TITLE_I_CH[dy][dx];
		if (col != 255) color(hConsole, col);
		if (ch == 32) cout << " ";
		else cout << char(ch);
	}
}

static const unsigned char TITLE_C_CH[3][208] = {
	{ 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 220, 220, 220, 220, 220, 32, 32, 32, 32, 32, 32, 32, 32, 220, 220, 220, 220, 220, 32, 220, 220, 220, 220, 220, 32, 220, 220, 220, 220, 220, 32, 220, 220, 220, 220, 220, 32, 32, 32, 32, 32, 32, 32, 32, 220, 220, 220, 220, 220, 32, 220, 220, 220, 220, 220, 32, 220, 220, 220, 220, 32, 32, 220, 220, 220, 220, 220, 32, 220, 220, 220, 220, 220, 32, 220, 220, 220, 220, 220, 32, 220, 32, 32, 32, 220, 32, 220, 220, 220, 220, 220, 32, 220, 220, 220, 220, 220, 32, 220, 220, 220, 220, 220, 32, 220, 220, 220, 220, 32, 32, 220, 32, 32, 32, 220, 32, 32, 32, 32, 220, 220, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32 },
	{ 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 219, 32, 220, 220, 219, 32, 32, 32, 32, 32, 32, 32, 32, 220, 220, 220, 220, 219, 32, 219, 32, 220, 223, 32, 32, 220, 220, 220, 220, 219, 32, 220, 220, 220, 220, 219, 32, 32, 32, 32, 32, 32, 32, 32, 219, 220, 220, 220, 219, 32, 219, 220, 220, 220, 219, 32, 219, 32, 32, 32, 219, 32, 219, 32, 220, 220, 220, 32, 219, 220, 220, 220, 219, 32, 219, 220, 220, 220, 219, 32, 219, 223, 220, 223, 219, 32, 219, 220, 220, 220, 219, 32, 219, 32, 32, 32, 32, 32, 32, 32, 219, 32, 32, 32, 219, 32, 32, 32, 219, 32, 219, 223, 220, 32, 219, 32, 32, 32, 32, 32, 219, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32 },
	{ 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 219, 220, 220, 220, 219, 32, 32, 32, 32, 32, 32, 32, 32, 219, 220, 220, 220, 220, 32, 219, 220, 220, 220, 219, 32, 219, 220, 220, 220, 220, 32, 219, 220, 220, 220, 220, 32, 32, 32, 32, 32, 32, 32, 32, 219, 32, 32, 32, 32, 32, 219, 32, 32, 223, 220, 32, 219, 220, 220, 220, 219, 32, 219, 220, 220, 220, 219, 32, 219, 32, 32, 223, 220, 32, 219, 32, 32, 32, 219, 32, 219, 32, 32, 32, 219, 32, 219, 32, 32, 32, 219, 32, 219, 220, 220, 220, 220, 32, 220, 220, 219, 220, 220, 32, 219, 220, 220, 220, 219, 32, 219, 32, 32, 223, 219, 32, 32, 32, 32, 32, 219, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32 },
};
static const unsigned char TITLE_C_COL[3][208] = {
	{ 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 11, 11, 11, 11, 11, 255, 255, 255, 255, 255, 255, 255, 255, 11, 11, 11, 11, 11, 255, 11, 11, 11, 11, 11, 255, 11, 11, 11, 11, 11, 255, 11, 11, 11, 11, 11, 255, 255, 255, 255, 255, 255, 255, 255, 11, 11, 11, 11, 11, 255, 11, 11, 11, 11, 11, 255, 11, 11, 11, 11, 11, 255, 11, 11, 11, 11, 11, 255, 11, 11, 11, 11, 11, 255, 11, 11, 11, 11, 11, 255, 11, 11, 11, 11, 11, 255, 11, 11, 11, 11, 11, 255, 11, 11, 11, 11, 11, 255, 11, 11, 11, 11, 11, 255, 11, 11, 11, 11, 11, 255, 11, 11, 11, 11, 11, 255, 255, 255, 255, 11, 11, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
	{ 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 11, 255, 11, 11, 11, 255, 255, 255, 255, 255, 255, 255, 255, 11, 11, 11, 11, 11, 255, 11, 255, 11, 11, 11, 255, 11, 11, 11, 11, 11, 255, 11, 11, 11, 11, 11, 255, 255, 255, 255, 255, 255, 255, 255, 11, 11, 11, 11, 11, 255, 11, 11, 11, 11, 11, 255, 11, 255, 255, 255, 11, 255, 11, 11, 11, 11, 11, 255, 11, 11, 11, 11, 11, 255, 11, 11, 11, 11, 11, 255, 11, 11, 11, 11, 11, 255, 11, 11, 11, 11, 11, 255, 11, 11, 11, 11, 11, 255, 11, 11, 11, 11, 11, 255, 11, 255, 255, 255, 11, 255, 11, 11, 11, 11, 11, 255, 255, 255, 255, 11, 11, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
	{ 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 2, 2, 2, 2, 2, 255, 255, 255, 255, 255, 255, 255, 255, 2, 2, 2, 2, 2, 255, 2, 2, 2, 2, 2, 255, 2, 2, 2, 2, 2, 255, 2, 2, 2, 2, 2, 255, 255, 255, 255, 255, 255, 255, 255, 2, 2, 2, 2, 2, 255, 2, 2, 2, 2, 2, 255, 2, 2, 2, 2, 2, 255, 2, 2, 2, 2, 2, 255, 2, 2, 2, 2, 2, 255, 2, 2, 2, 2, 2, 255, 2, 2, 2, 2, 2, 255, 2, 2, 2, 2, 2, 255, 2, 2, 2, 2, 2, 255, 2, 2, 2, 2, 2, 255, 2, 2, 2, 2, 2, 255, 2, 2, 2, 2, 2, 255, 255, 255, 255, 2, 2, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
};
void C_2022_Programaci�n1(int& X, int& Y) {
	if (X < 2 || X > 209 || Y < 45 || Y > 47) {
		cout << " ";
		return;
	}
	{
		int dx = X - 2, dy = Y - 45;
		unsigned char col = TITLE_C_COL[dy][dx];
		unsigned char ch = TITLE_C_CH[dy][dx];
		if (col != 255) color(hConsole, col);
		if (ch == 32) cout << " ";
		else cout << char(ch);
	}
}

void Dibuja_OJugar(int& X, int& Y) {
	// Jugar
	if (X == 91 && Y == 27) {  //  J
		cout << " ";
	}
	else
		if (X == 91 && Y == 28) {
			cout << char(220);
		}
		else
			if (X == 91 && Y == 29) {
				cout << " ";
			}
			else
				if (X == 92 && Y == 27) {
					cout << " ";
				}
				else
					if (X == 92 && Y == 28) {
						cout << " ";
					}
					else
						if (X == 92 && Y == 29) {
							cout << char(223);
						}
						else
							if (X == 93 && Y == 27) {
								cout << " ";
							}
							else
								if (X == 93 && Y == 28) {
									cout << " ";
								}
								else
									if (X == 93 && Y == 29) {
										cout << char(223);
									}
									else
										if (X == 94 && Y == 27) {
											cout << " ";
										}
										else
											if (X == 94 && Y == 28) {
												cout << " ";
											}
											else
												if (X == 94 && Y == 29) {
													cout << char(223);
												}
												else
													if (X == 95 && Y == 27) {
														cout << char(219);
													}
													else
														if (X == 95 && Y == 28) {
															cout << char(219);
														}
														else
															if (X == 95 && Y == 29) {
																cout << " ";
															}
															else  //  Fin J
																if ((X == 97 || X == 101) && Y == 27) {  //  U
																	cout << char(219);
																}
																else
																	if ((X == 97 || X == 101) && Y == 28) {
																		cout << char(219);
																	}
																	else
																		if ((X == 97 || X == 101) && Y == 29) {
																			cout << " ";
																		}
																		else
																			if ((X == 98 || X == 99 || X == 100) && Y == 27) {
																				cout << " ";
																			}
																			else
																				if ((X == 98 || X == 99 || X == 100) && Y == 28) {
																					cout << " ";
																				}
																				else
																					if ((X == 98 || X == 99 || X == 100) && Y == 29) {
																						cout << char(223);
																					}
																					else//  Fin U
																						if (X == 103 && Y == 27) {  //  G
																							cout << char(220);
																						}
																						else
																							if (X == 103 && Y == 28) {
																								cout << char(219);
																							}
																							else
																								if (X == 103 && Y == 29) {
																									cout << " ";
																								}
																								else
																									if ((X == 104 || X == 105) && Y == 27) {
																										cout << char(223);
																									}
																									else
																										if ((X == 104 || X == 105) && Y == 28) {
																											cout << " ";
																										}
																										else
																											if ((X == 104 || X == 105) && Y == 29) {
																												cout << char(223);
																											}
																											else
																												if (X == 106 && Y == 27) {
																													cout << char(223);
																												}
																												else
																													if (X == 106 && Y == 28) {
																														cout << char(223);
																													}
																													else
																														if (X == 106 && Y == 29) {
																															cout << char(223);
																														}
																														else
																															if (X == 107 && Y == 27) {
																																cout << " ";
																															}
																															else
																																if (X == 107 && Y == 28) {
																																	cout << char(219);
																																}
																																else
																																	if (X == 107 && Y == 29) {
																																		cout << " ";
																																	}
																																	else  //  Fin G
																																		if ((X == 109 || X == 113) && Y == 27) {  //  A
																																			cout << char(220);
																																		}
																																		else
																																			if ((X == 109 || X == 113) && Y == 28) {
																																				cout << char(219);
																																			}
																																			else
																																				if ((X == 109 || X == 113) && Y == 29) {
																																					cout << char(223);
																																				}
																																				else
																																					if ((X == 110 || X == 111 || X == 112) && Y == 27) {
																																						cout << char(223);
																																					}
																																					else
																																						if ((X == 110 || X == 111 || X == 112) && Y == 28) {
																																							cout << char(223);
																																						}
																																						else
																																							if ((X == 110 || X == 111 || X == 112) && Y == 29) {
																																								cout << " ";
																																							}
																																							else  // Fin A
																																								if (X == 115 && Y == 27) {  //  R	
																																									cout << char(219);
																																								}
																																								else
																																									if (X == 115 && Y == 28) {
																																										cout << char(219);
																																									}
																																									else
																																										if (X == 115 && Y == 29) {
																																											cout << char(223);
																																										}
																																										else
																																											if ((X == 116 || X == 117) && Y == 27) {
																																												cout << char(223);
																																											}
																																											else
																																												if ((X == 116 || X == 117) && Y == 28) {
																																													cout << char(223);
																																												}
																																												else
																																													if ((X == 116 || X == 117) && Y == 29) {
																																														cout << " ";
																																													}
																																													else
																																														if (X == 118 && Y == 27) {
																																															cout << char(223);
																																														}
																																														else
																																															if (X == 118 && Y == 28) {
																																																cout << char(219);
																																															}
																																															else
																																																if (X == 118 && Y == 29) {
																																																	cout << " ";
																																																}
																																																else
																																																	if (X == 119 && Y == 27) {
																																																		cout << char(220);
																																																	}
																																																	else
																																																		if (X == 119 && Y == 28) {
																																																			cout << char(223);
																																																		}
																																																		else
																																																			if (X == 119 && Y == 29) {
																																																				cout << char(223);
																																																			}
	//  Fin R
}

void Dibuja_OOpciones(int& X, int& Y) {
	//  Opciones
	if ((X == 82 || X == 86) && Y == 31) {  //  O
		cout << char(220);
	}
	else
		if ((X == 82 || X == 86) && Y == 32) {
			cout << char(219);
		}
		else
			if ((X == 82 || X == 86) && Y == 33) {
				cout << " ";
			}
			else
				if ((X == 83 || X == 84 || X == 85) && Y == 31) {
					cout << char(223);
				}
				else
					if ((X == 83 || X == 84 || X == 85) && Y == 32) {
						cout << " ";
					}
					else
						if ((X == 83 || X == 84 || X == 85) && Y == 33) {
							cout << char(223);
						}
						else  //  Fin O
							if (X == 88 && Y == 31) {  //  P
								cout << char(219);
							}
							else
								if (X == 88 && Y == 32) {
									cout << char(219);
								}
								else
									if (X == 88 && Y == 33) {
										cout << char(223);
									}
									else
										if ((X == 89 || X == 90 || X == 91) && Y == 31) {
											cout << char(223);
										}
										else
											if ((X == 89 || X == 90 || X == 91) && Y == 32) {
												cout << char(223);
											}
											else
												if ((X == 89 || X == 90 || X == 91) && Y == 33) {
													cout << " ";
												}
												else
													if (X == 92 && Y == 31) {
														cout << char(220);
													}
													else
														if (X == 92 && Y == 32) {
															cout << " ";
														}
														else
															if (X == 92 && Y == 33) {
																cout << " ";
															}
															else  //  Fin P
																if (X == 94 && Y == 31) {  //  C
																	cout << char(220);
																}
																else
																	if (X == 94 && Y == 32) {
																		cout << char(219);
																	}
																	else
																		if (X == 94 && Y == 33) {
																			cout << " ";
																		}
																		else
																			if ((X == 95 || X == 96 || X == 97) && Y == 31) {
																				cout << char(223);
																			}
																			else
																				if ((X == 95 || X == 96 || X == 97) && Y == 32) {
																					cout << " ";
																				}
																				else
																					if ((X == 95 || X == 96 || X == 97) && Y == 33) {
																						cout << char(223);
																					}
																					else
																						if (X == 98 && Y == 31) {
																							cout << char(220);
																						}
																						else
																							if (X == 98 && Y == 32) {
																								cout << char(220);
																							}
																							else
																								if (X == 98 && Y == 33) {
																									cout << " ";
																								}
																								else  //  Fin C

																									if ((X == 100 || X == 101 || X == 103 || X == 104) && Y == 31) {  //  I
																										cout << char(223);
																									}
																									else
																										if ((X == 100 || X == 101 || X == 103 || X == 104) && Y == 32) {
																											cout << " ";
																										}
																										else
																											if ((X == 100 || X == 101 || X == 103 || X == 104) && Y == 33) {
																												cout << char(223);
																											}
																											else
																												if (X == 102 && Y == 31) {
																													cout << char(219);
																												}
																												else
																													if (X == 102 && Y == 32) {
																														cout << char(219);
																													}
																													else
																														if (X == 102 && Y == 33) {
																															cout << char(223);
																														}
																														else  //  Fin I
																															if ((X == 106 || X == 110) && Y == 31) {  //  O
																																cout << char(220);
																															}
																															else
																																if ((X == 106 || X == 110) && Y == 32) {
																																	cout << char(219);
																																}
																																else
																																	if ((X == 106 || X == 110) && Y == 33) {
																																		cout << " ";
																																	}
																																	else
																																		if ((X == 107 || X == 108 || X == 109) && Y == 31) {
																																			cout << char(223);
																																		}
																																		else
																																			if ((X == 107 || X == 108 || X == 109) && Y == 32) {
																																				cout << " ";
																																			}
																																			else
																																				if ((X == 107 || X == 108 || X == 109) && Y == 33) {
																																					cout << char(223);
																																				}
																																				else  //  Fin O
																																					if ((X == 112 || X == 116) && Y == 31) {  //  N
																																						cout << char(219);
																																					}
																																					else
																																						if ((X == 112 || X == 116) && Y == 32) {
																																							cout << char(219);
																																						}
																																						else
																																							if ((X == 112 || X == 116) && Y == 33) {
																																								cout << char(223);
																																							}
																																							else
																																								if (X == 113 && Y == 31) {
																																									cout << char(220);
																																								}
																																								else
																																									if (X == 113 && Y == 32) {
																																										cout << " ";
																																									}
																																									else
																																										if (X == 113 && Y == 33) {
																																											cout << " ";
																																										}
																																										else
																																											if (X == 114 && Y == 31) {
																																												cout << " ";
																																											}
																																											else
																																												if (X == 114 && Y == 32) {
																																													cout << char(223);
																																												}
																																												else
																																													if (X == 114 && Y == 33) {
																																														cout << " ";
																																													}
																																													else
																																														if (X == 115 && Y == 31) {
																																															cout << " ";
																																														}
																																														else
																																															if (X == 115 && Y == 32) {
																																																cout << char(220);
																																															}
																																															else
																																																if (X == 115 && Y == 33) {
																																																	cout << " ";
																																																}
																																																else  //  Fin N
																																																	if (X == 118 && Y == 31) {  //  E
																																																		cout << char(219);
																																																	}
																																																	else
																																																		if (X == 118 && Y == 32) {
																																																			cout << char(219);
																																																		}
																																																		else
																																																			if (X == 118 && Y == 33) {
																																																				cout << char(223);
																																																			}
																																																			else
																																																				if ((X == 119 || X == 120 || X == 121) && Y == 31) {
																																																					cout << char(223);
																																																				}
																																																				else
																																																					if ((X == 119 || X == 120 || X == 121) && Y == 32) {
																																																						cout << char(223);
																																																					}
																																																					else
																																																						if ((X == 119 || X == 120 || X == 121) && Y == 33) {
																																																							cout << char(223);
																																																						}
																																																						else
																																																							if (X == 122 && Y == 31) {
																																																								cout << char(223);
																																																							}
																																																							else
																																																								if (X == 122 && Y == 32) {
																																																									cout << " ";
																																																								}
																																																								else
																																																									if (X == 122 && Y == 33) {
																																																										cout << char(223);
																																																									}
																																																									else  //  Fin E
																																																										if (X == 124 && Y == 31) {  //  S
																																																											cout << char(220);
																																																										}
																																																										else
																																																											if (X == 124 && Y == 32) {
																																																												cout << " ";
																																																											}
																																																											else
																																																												if (X == 124 && Y == 33) {
																																																													cout << char(223);
																																																												}
																																																												else
																																																													if ((X == 125 || X == 126 || X == 127) && Y == 31) {
																																																														cout << char(223);
																																																													}
																																																													else
																																																														if ((X == 125 || X == 126 || X == 127) && Y == 32) {
																																																															cout << char(223);
																																																														}
																																																														else
																																																															if ((X == 125 || X == 126 || X == 127) && Y == 33) {
																																																																cout << char(223);
																																																															}
																																																															else
																																																																if (X == 128 && Y == 31) {
																																																																	cout << char(223);
																																																																}
																																																																else
																																																																	if (X == 128 && Y == 32) {
																																																																		cout << char(220);
																																																																	}
																																																																	else
																																																																		if (X == 128 && Y == 33) {
																																																																			cout << " ";
																																																																		}
	//  Fin S



}

void Dibuja_OInstrucciones(int& X, int& Y) {
	//  Instrucciones
	if ((X == 67 || X == 68 || X == 70 || X == 71) && Y == 35) {  //  I
		cout << char(223);
	}
	else
		if ((X == 67 || X == 68 || X == 70 || X == 71) && Y == 36) {
			cout << " ";
		}
		else
			if ((X == 67 || X == 68 || X == 70 || X == 71) && Y == 37) {
				cout << char(223);
			}
			else
				if (X == 69 && Y == 35) {
					cout << char(219);
				}
				else
					if (X == 69 && Y == 36) {
						cout << char(219);
					}
					else
						if (X == 69 && Y == 37) {
							cout << char(223);
						}
						else
							if ((X == 73 || X == 77) && Y == 35) {  //  N
								cout << char(219);
							}
							else
								if ((X == 73 || X == 77) && Y == 36) {
									cout << char(219);
								}
								else
									if ((X == 73 || X == 77) && Y == 37) {
										cout << char(223);
									}
									else
										if (X == 74 && Y == 35) {
											cout << char(220);
										}
										else
											if (X == 74 && Y == 36) {
												cout << " ";
											}
											else
												if (X == 74 && Y == 37) {
													cout << " ";
												}
												else
													if (X == 75 && Y == 35) {
														cout << " ";
													}
													else
														if (X == 75 && Y == 36) {
															cout << char(223);
														}
														else
															if (X == 75 && Y == 37) {
																cout << " ";
															}
															else
																if (X == 76 && Y == 35) {
																	cout << " ";
																}
																else
																	if (X == 76 && Y == 36) {
																		cout << char(220);
																	}
																	else
																		if (X == 76 && Y == 37) {
																			cout << " ";
																		}
																		else  //  Fin N
																			if (X == 79 && Y == 35) {  //  S
																				cout << char(220);
																			}
																			else
																				if (X == 79 && Y == 36) {
																					cout << " ";
																				}
																				else
																					if (X == 79 && Y == 37) {
																						cout << char(223);
																					}
																					else
																						if ((X == 80 || X == 81 || X == 82) && Y == 35) {
																							cout << char(223);
																						}
																						else
																							if ((X == 80 || X == 81 || X == 82) && Y == 36) {
																								cout << char(223);
																							}
																							else
																								if ((X == 80 || X == 81 || X == 82) && Y == 37) {
																									cout << char(223);
																								}
																								else
																									if (X == 83 && Y == 35) {
																										cout << char(223);
																									}
																									else
																										if (X == 83 && Y == 36) {
																											cout << char(220);
																										}
																										else
																											if (X == 83 && Y == 37) {
																												cout << " ";
																											}
																											else  //  Fin S
																												if ((X == 85 || X == 86 || X == 88 || X == 89) && Y == 35) {  //  T
																													cout << char(223);
																												}
																												else
																													if ((X == 85 || X == 86 || X == 88 || X == 89) && Y == 36) {
																														cout << " ";
																													}
																													else
																														if ((X == 85 || X == 86 || X == 88 || X == 89) && Y == 37) {
																															cout << " ";
																														}
																														else
																															if (X == 87 && Y == 35) {
																																cout << char(219);
																															}
																															else
																																if (X == 87 && Y == 36) {
																																	cout << char(219);
																																}
																																else
																																	if (X == 87 && Y == 37) {
																																		cout << char(223);
																																	}
																																	else  //  Fin T
																																		if (X == 91 && Y == 35) {  //  R	
																																			cout << char(219);
																																		}
																																		else
																																			if (X == 91 && Y == 36) {
																																				cout << char(219);
																																			}
																																			else
																																				if (X == 91 && Y == 37) {
																																					cout << char(223);
																																				}
																																				else
																																					if ((X == 92 || X == 93) && Y == 35) {
																																						cout << char(223);
																																					}
																																					else
																																						if ((X == 92 || X == 93) && Y == 36) {
																																							cout << char(223);
																																						}
																																						else
																																							if ((X == 92 || X == 93) && Y == 37) {
																																								cout << " ";
																																							}
																																							else
																																								if (X == 94 && Y == 35) {
																																									cout << char(223);
																																								}
																																								else
																																									if (X == 94 && Y == 36) {
																																										cout << char(219);
																																									}
																																									else
																																										if (X == 94 && Y == 37) {
																																											cout << " ";
																																										}
																																										else
																																											if (X == 95 && Y == 35) {
																																												cout << char(220);
																																											}
																																											else
																																												if (X == 95 && Y == 36) {
																																													cout << char(223);
																																												}
																																												else
																																													if (X == 95 && Y == 37) {
																																														cout << char(223);
																																													}
																																													else  //  Fin R
																																														if ((X == 97 || X == 101) && Y == 35) {  //  U
																																															cout << char(219);
																																														}
																																														else
																																															if ((X == 97 || X == 101) && Y == 36) {
																																																cout << char(219);
																																															}
																																															else
																																																if ((X == 97 || X == 101) && Y == 37) {
																																																	cout << " ";
																																																}
																																																else
																																																	if ((X == 98 || X == 99 || X == 100) && Y == 35) {
																																																		cout << " ";
																																																	}
																																																	else
																																																		if ((X == 98 || X == 99 || X == 100) && Y == 36) {
																																																			cout << " ";
																																																		}
																																																		else
																																																			if ((X == 98 || X == 99 || X == 100) && Y == 37) {
																																																				cout << char(223);
																																																			}
																																																			else//  Fin U
																																																				if (X == 103 && Y == 35) {  //  C
																																																					cout << char(220);
																																																				}
																																																				else
																																																					if (X == 103 && Y == 36) {
																																																						cout << char(219);
																																																					}
																																																					else
																																																						if (X == 103 && Y == 37) {
																																																							cout << " ";
																																																						}
																																																						else
																																																							if ((X == 104 || X == 105 || X == 106) && Y == 35) {
																																																								cout << char(223);
																																																							}
																																																							else
																																																								if ((X == 104 || X == 105 || X == 106) && Y == 36) {
																																																									cout << " ";
																																																								}
																																																								else
																																																									if ((X == 104 || X == 105 || X == 106) && Y == 37) {
																																																										cout << char(223);
																																																									}
																																																									else
																																																										if (X == 107 && Y == 35) {
																																																											cout << char(220);
																																																										}
																																																										else
																																																											if (X == 107 && Y == 36) {
																																																												cout << char(220);
																																																											}
																																																											else
																																																												if (X == 107 && Y == 37) {
																																																													cout << " ";
																																																												}
																																																												else  //  Fin C
																																																													if (X == 109 && Y == 35) {  //  C
																																																														cout << char(220);
																																																													}
																																																													else
																																																														if (X == 109 && Y == 36) {
																																																															cout << char(219);
																																																														}
																																																														else
																																																															if (X == 109 && Y == 37) {
																																																																cout << " ";
																																																															}
																																																															else
																																																																if ((X == 110 || X == 111 || X == 112) && Y == 35) {
																																																																	cout << char(223);
																																																																}
																																																																else
																																																																	if ((X == 110 || X == 111 || X == 112) && Y == 36) {
																																																																		cout << " ";
																																																																	}
																																																																	else
																																																																		if ((X == 110 || X == 111 || X == 112) && Y == 37) {
																																																																			cout << char(223);
																																																																		}
																																																																		else
																																																																			if (X == 113 && Y == 35) {
																																																																				cout << char(220);
																																																																			}
																																																																			else
																																																																				if (X == 113 && Y == 36) {
																																																																					cout << char(220);
																																																																				}
																																																																				else
																																																																					if (X == 113 && Y == 37) {
																																																																						cout << " ";
																																																																					}
																																																																					else  //  Fin C
																																																																						if ((X == 115 || X == 116 || X == 118 || X == 119) && Y == 35) {  //  I
																																																																							cout << char(223);
																																																																						}
																																																																						else
																																																																							if ((X == 115 || X == 116 || X == 118 || X == 119) && Y == 36) {
																																																																								cout << " ";
																																																																							}
																																																																							else
																																																																								if ((X == 115 || X == 116 || X == 118 || X == 119) && Y == 37) {
																																																																									cout << char(223);
																																																																								}
																																																																								else
																																																																									if (X == 117 && Y == 35) {
																																																																										cout << char(219);
																																																																									}
																																																																									else
																																																																										if (X == 117 && Y == 36) {
																																																																											cout << char(219);
																																																																										}
																																																																										else
																																																																											if (X == 117 && Y == 37) {
																																																																												cout << char(223);
																																																																											}
																																																																											else  //  Fin I
																																																																												if ((X == 121 || X == 125) && Y == 35) {  //  O
																																																																													cout << char(220);
																																																																												}
																																																																												else
																																																																													if ((X == 121 || X == 125) && Y == 36) {
																																																																														cout << char(219);
																																																																													}
																																																																													else
																																																																														if ((X == 121 || X == 125) && Y == 37) {
																																																																															cout << " ";
																																																																														}
																																																																														else
																																																																															if ((X == 122 || X == 123 || X == 124) && Y == 35) {
																																																																																cout << char(223);
																																																																															}
																																																																															else
																																																																																if ((X == 122 || X == 123 || X == 124) && Y == 36) {
																																																																																	cout << " ";
																																																																																}
																																																																																else
																																																																																	if ((X == 122 || X == 123 || X == 124) && Y == 37) {
																																																																																		cout << char(223);
																																																																																	}
																																																																																	else  //  Fin O
																																																																																		if ((X == 127 || X == 131) && Y == 35) {  //  N
																																																																																			cout << char(219);
																																																																																		}
																																																																																		else
																																																																																			if ((X == 127 || X == 131) && Y == 36) {
																																																																																				cout << char(219);
																																																																																			}
																																																																																			else
																																																																																				if ((X == 127 || X == 131) && Y == 37) {
																																																																																					cout << char(223);
																																																																																				}
																																																																																				else
																																																																																					if (X == 128 && Y == 35) {
																																																																																						cout << char(220);
																																																																																					}
																																																																																					else
																																																																																						if (X == 128 && Y == 36) {
																																																																																							cout << " ";
																																																																																						}
																																																																																						else
																																																																																							if (X == 128 && Y == 37) {
																																																																																								cout << " ";
																																																																																							}
																																																																																							else
																																																																																								if (X == 129 && Y == 35) {
																																																																																									cout << " ";
																																																																																								}
																																																																																								else
																																																																																									if (X == 129 && Y == 36) {
																																																																																										cout << char(223);
																																																																																									}
																																																																																									else
																																																																																										if (X == 129 && Y == 37) {
																																																																																											cout << " ";
																																																																																										}
																																																																																										else
																																																																																											if (X == 130 && Y == 35) {
																																																																																												cout << " ";
																																																																																											}
																																																																																											else
																																																																																												if (X == 130 && Y == 36) {
																																																																																													cout << char(220);
																																																																																												}
																																																																																												else
																																																																																													if (X == 130 && Y == 37) {
																																																																																														cout << " ";
																																																																																													}
																																																																																													else  //  Fin N
																																																																																														if (X == 133 && Y == 35) {  //  E
																																																																																															cout << char(219);
																																																																																														}
																																																																																														else
																																																																																															if (X == 133 && Y == 36) {
																																																																																																cout << char(219);
																																																																																															}
																																																																																															else
																																																																																																if (X == 133 && Y == 37) {
																																																																																																	cout << char(223);
																																																																																																}
																																																																																																else
																																																																																																	if ((X == 134 || X == 135 || X == 136) && Y == 35) {
																																																																																																		cout << char(223);
																																																																																																	}
																																																																																																	else
																																																																																																		if ((X == 134 || X == 135 || X == 136) && Y == 36) {
																																																																																																			cout << char(223);
																																																																																																		}
																																																																																																		else
																																																																																																			if ((X == 134 || X == 135 || X == 136) && Y == 37) {
																																																																																																				cout << char(223);
																																																																																																			}
																																																																																																			else
																																																																																																				if (X == 137 && Y == 35) {
																																																																																																					cout << char(223);
																																																																																																				}
																																																																																																				else
																																																																																																					if (X == 137 && Y == 36) {
																																																																																																						cout << " ";
																																																																																																					}
																																																																																																					else
																																																																																																						if (X == 137 && Y == 37) {
																																																																																																							cout << char(223);
																																																																																																						}
																																																																																																						else  //  Fin E
																																																																																																							if (X == 139 && Y == 35) {  //  S
																																																																																																								cout << char(220);
																																																																																																							}
																																																																																																							else
																																																																																																								if (X == 139 && Y == 36) {
																																																																																																									cout << " ";
																																																																																																								}
																																																																																																								else
																																																																																																									if (X == 139 && Y == 37) {
																																																																																																										cout << char(223);
																																																																																																									}
																																																																																																									else
																																																																																																										if ((X == 140 || X == 141 || X == 142) && Y == 35) {
																																																																																																											cout << char(223);
																																																																																																										}
																																																																																																										else
																																																																																																											if ((X == 140 || X == 141 || X == 142) && Y == 36) {
																																																																																																												cout << char(223);
																																																																																																											}
																																																																																																											else
																																																																																																												if ((X == 140 || X == 141 || X == 142) && Y == 37) {
																																																																																																													cout << char(223);
																																																																																																												}
																																																																																																												else
																																																																																																													if (X == 143 && Y == 35) {
																																																																																																														cout << char(223);
																																																																																																													}
																																																																																																													else
																																																																																																														if (X == 143 && Y == 36) {
																																																																																																															cout << char(220);
																																																																																																														}
																																																																																																														else
																																																																																																															if (X == 143 && Y == 37) {
																																																																																																																cout << " ";
																																																																																																															}
	//  Fin S

}

void Dibuja_OSalir(int& X, int& Y) {
	//  Salir
	if (X == 91 && Y == 39) {  //  S
		cout << char(220);
	}
	else
		if (X == 91 && Y == 40) {
			cout << " ";
		}
		else
			if (X == 91 && Y == 41) {
				cout << char(223);
			}
			else
				if ((X == 92 || X == 93 || X == 94) && Y == 39) {
					cout << char(223);
				}
				else
					if ((X == 92 || X == 93 || X == 94) && Y == 40) {
						cout << char(223);
					}
					else
						if ((X == 92 || X == 93 || X == 94) && Y == 41) {
							cout << char(223);
						}
						else
							if (X == 95 && Y == 39) {
								cout << char(223);
							}
							else
								if (X == 95 && Y == 40) {
									cout << char(220);
								}
								else
									if (X == 95 && Y == 41) {
										cout << " ";
									}
									else  //  Fin S
										if ((X == 97 || X == 101) && Y == 39) {  //  A
											cout << char(220);
										}
										else
											if ((X == 97 || X == 101) && Y == 40) {
												cout << char(219);
											}
											else
												if ((X == 97 || X == 101) && Y == 41) {
													cout << char(223);
												}
												else
													if ((X == 98 || X == 99 || X == 100) && Y == 39) {
														cout << char(223);
													}
													else
														if ((X == 98 || X == 99 || X == 100) && Y == 40) {
															cout << char(223);
														}
														else
															if ((X == 98 || X == 99 || X == 100) && Y == 41) {
																cout << " ";
															}
															else  // Fin A
																if (X == 103 && Y == 39) {  //  L
																	cout << char(219);
																}
																else
																	if (X == 103 && Y == 40) {
																		cout << char(219);
																	}
																	else
																		if (X == 103 && Y == 41) {
																			cout << char(223);
																		}
																		else
																			if ((X == 104 || X == 105 || X == 106 || X == 107) && Y == 39) {
																				cout << " ";
																			}
																			else
																				if ((X == 104 || X == 105 || X == 106 || X == 107) && Y == 40) {
																					cout << " ";
																				}
																				else
																					if ((X == 104 || X == 105 || X == 106 || X == 107) && Y == 41) {
																						cout << char(223);
																					}
																					else  //  Fin L
																						if ((X == 109 || X == 110 || X == 112 || X == 113) && Y == 39) {  //  I
																							cout << char(223);
																						}
																						else
																							if ((X == 109 || X == 110 || X == 112 || X == 113) && Y == 40) {
																								cout << " ";
																							}
																							else
																								if ((X == 109 || X == 110 || X == 112 || X == 113) && Y == 41) {
																									cout << char(223);
																								}
																								else
																									if (X == 111 && Y == 39) {
																										cout << char(219);
																									}
																									else
																										if (X == 111 && Y == 40) {
																											cout << char(219);
																										}
																										else
																											if (X == 111 && Y == 41) {
																												cout << char(223);
																											}
																											else  //  Fin I
																												if (X == 115 && Y == 39) {  //  R	
																													cout << char(219);
																												}
																												else
																													if (X == 115 && Y == 40) {
																														cout << char(219);
																													}
																													else
																														if (X == 115 && Y == 41) {
																															cout << char(223);
																														}
																														else
																															if ((X == 116 || X == 117) && Y == 39) {
																																cout << char(223);
																															}
																															else
																																if ((X == 116 || X == 117) && Y == 40) {
																																	cout << char(223);
																																}
																																else
																																	if ((X == 116 || X == 117) && Y == 41) {
																																		cout << " ";
																																	}
																																	else
																																		if (X == 118 && Y == 39) {
																																			cout << char(223);
																																		}
																																		else
																																			if (X == 118 && Y == 40) {
																																				cout << char(219);
																																			}
																																			else
																																				if (X == 118 && Y == 41) {
																																					cout << " ";
																																				}
																																				else
																																					if (X == 119 && Y == 39) {
																																						cout << char(220);
																																					}
																																					else
																																						if (X == 119 && Y == 40) {
																																							cout << char(223);
																																						}
																																						else
																																							if (X == 119 && Y == 41) {
																																								cout << char(223);
																																							}
	//  Fin R
}

void Dibuja_Flechita(int& X, int& Y, int* YFlch, int* XFlch, int* ColorL) {
	color(hConsole, 15);
	if (*XFlch == 83 && *YFlch == 27) {
		if ((X == 83 || X == 127) && (Y == 27 || Y == 29)) {
			cout << char(223);
		}
		else
			if ((X == 82 || X == 128) && (Y == 27 || Y == 28)) {
				cout << char(219);
			}
			else
				if ((X == 82 || X == 128) && Y == 29) {
					cout << char(223);
				}
				else
					cout << " ";
	}
	else
		if (*XFlch == 74 && *YFlch == 31) {
			if ((X == 74 || X == 136) && (Y == 31 || Y == 33)) {
				cout << char(223);
			}
			else
				if ((X == 73 || X == 137) && (Y == 31 || Y == 32)) {
					cout << char(219);
				}
				else
					if ((X == 73 || X == 137) && Y == 33) {
						cout << char(223);
					}
					else
						cout << " ";
		}
		else
			if (*XFlch == 59 && *YFlch == 35) {
				if ((X == 59 || X == 151) && (Y == 35 || Y == 37)) {
					cout << char(223);
				}
				else
					if ((X == 58 || X == 152) && (Y == 35 || Y == 36)) {
						cout << char(219);
					}
					else
						if ((X == 58 || X == 152) && Y == 37) {
							cout << char(223);
						}
						else
							cout << " ";
			}
			else
				if (*XFlch == 83 && *YFlch == 39) {
					if ((X == 83 || X == 127) && (Y == 39 || Y == 41)) {
						cout << char(223);
					}
					else
						if ((X == 82 || X == 128) && (Y == 39 || Y == 40)) {
							cout << char(219);
						}
						else
							if ((X == 82 || X == 128) && Y == 41) {
								cout << char(223);
							}
							else
								cout << " ";
				}
				else
					if (*XFlch == 1 && *YFlch == 1) {
						if ((X == 83 || X == 127) && (Y == 27 || Y == 29)) {
							cout << " ";
						}
						else
							if ((X == 82 || X == 128) && (Y == 27 || Y == 28)) {
								cout << " ";
							}
							else
								if ((X == 82 || X == 128) && Y == 29) {
									cout << " ";
								}
								else
									if ((X == 74 || X == 136) && (Y == 31 || Y == 33)) {
										cout << " ";
									}
									else
										if ((X == 73 || X == 137) && (Y == 31 || Y == 32)) {
											cout << " ";
										}
										else
											if ((X == 73 || X == 137) && Y == 33) {
												cout << " ";
											}
											else
												if ((X == 59 || X == 151) && (Y == 35 || Y == 37)) {
													cout << " ";
												}
												else
													if ((X == 58 || X == 152) && (Y == 35 || Y == 36)) {
														cout << " ";
													}
													else
														if ((X == 58 || X == 152) && Y == 37) {
															cout << " ";
														}
														else
															if ((X == 83 || X == 127) && (Y == 39 || Y == 41)) {
																cout << " ";
															}
															else
																if ((X == 82 || X == 128) && (Y == 39 || Y == 40)) {
																	cout << " ";
																}
																else
																	if ((X == 82 || X == 128) && Y == 41) {
																		cout << " ";
																	}
																	else
																		cout << " ";
					}
					else
						if (*XFlch == 69 && *YFlch == 32) {
							if (X == 69 && (Y == 32 || Y == 34)) {
								cout << char(223);
							}
							else
								if (X == 68 && (Y == 32 || Y == 33)) {
									cout << char(219);
								}
								else
									if (X == 68 && Y == 34) {
										cout << char(223);
									}
									else
										if (X == 95 && (Y == 32 || Y == 34)) {
											cout << char(223);
										}
										else
											if (X == 96 && (Y == 32 || Y == 33)) {
												cout << char(219);
											}
											else
												if (X == 96 && Y == 34) {
													cout << char(223);
												}

						}
						else
							if (*XFlch == 116 && *YFlch == 32) {
								if (X == 116 && (Y == 32 || Y == 34)) {
									cout << char(223);
								}
								else
									if (X == 115 && (Y == 32 || Y == 33)) {
										cout << char(219);
									}
									else
										if (X == 115 && Y == 34) {
											cout << char(223);
										}
										else
											if (X == 142 && (Y == 32 || Y == 34)) {
												cout << char(223);
											}
											else
												if (X == 143 && (Y == 32 || Y == 33)) {
													cout << char(219);
												}
												else
													if (X == 143 && Y == 34) {
														cout << char(223);
													}
							}
							else
								cout << "";

}

void Jugar(int& X, int& Y, int* ColorL, short& cont, char* Nombre, short& contL) {
	if (dibujarTecladoNombre(X, Y, ColorL)) {
		return;
	}

	if (contL == 0) {
		if (glifoEnBBoxY(136, 7, X, Y)) {
			dibujarSprite5x3(SPR_P1, 136, 7, X, Y, 1 + rand() % 15);
		}
		else {
			dibujarNombreIngresado(X, Y, cont, Nombre);
		}
	}
	else if (contL == 1) {
		if (glifoEnBBoxY(136, 7, X, Y)) {
			dibujarSprite5x3(SPR_P2, 136, 7, X, Y, 1 + rand() % 15);
		}
		else {
			dibujarNombreIngresado(X, Y, cont, Nombre);
		}
	}
	else {
		cout << " ";
	}
}

void LetrasJ_A(int& X, int& Y, short& cont, char* Nombre) {
	dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void LetrasJ_B(int& X, int& Y, short& cont, char* Nombre) {
	dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void LetrasJ_C(int& X, int& Y, short& cont, char* Nombre) {
	dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void LetrasJ_D(int& X, int& Y, short& cont, char* Nombre) {
	dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void LetrasJ_E(int& X, int& Y, short& cont, char* Nombre) {
	dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void LetrasJ_F(int& X, int& Y, short& cont, char* Nombre) {
	dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void LetrasJ_G(int& X, int& Y, short& cont, char* Nombre) {
	dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void LetrasJ_H(int& X, int& Y, short& cont, char* Nombre) {
	dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void LetrasJ_I(int& X, int& Y, short& cont, char* Nombre) {
	dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void LetrasJ_J(int& X, int& Y, short& cont, char* Nombre) {
	dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void LetrasJ_K(int& X, int& Y, short& cont, char* Nombre) {
	dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void LetrasJ_L(int& X, int& Y, short& cont, char* Nombre) {
	dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void LetrasJ_M(int& X, int& Y, short& cont, char* Nombre) {
	dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void LetrasJ_N(int& X, int& Y, short& cont, char* Nombre) {
	dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void LetrasJ_O(int& X, int& Y, short& cont, char* Nombre) {
	dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void LetrasJ_P(int& X, int& Y, short& cont, char* Nombre) {
	dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void LetrasJ_Q(int& X, int& Y, short& cont, char* Nombre) {
	dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void LetrasJ_R(int& X, int& Y, short& cont, char* Nombre) {
	dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void LetrasJ_S(int& X, int& Y, short& cont, char* Nombre) {
	dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void LetrasJ_T(int& X, int& Y, short& cont, char* Nombre) {
	dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void LetrasJ_U(int& X, int& Y, short& cont, char* Nombre) {
	dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void LetrasJ_V(int& X, int& Y, short& cont, char* Nombre) {
	dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void LetrasJ_W(int& X, int& Y, short& cont, char* Nombre) {
	dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void LetrasJ_X(int& X, int& Y, short& cont, char* Nombre) {
	dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void LetrasJ_Y(int& X, int& Y, short& cont, char* Nombre) {
	dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void LetrasJ_Z(int& X, int& Y, short& cont, char* Nombre) {
	dibujarDigrama(Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void Letras3J_A_D(int& X, int& Y, short& cont, char* Nombre) {
	dibujarTrigrama(Nombre[cont - 3], Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void Letras3J_E_H(int& X, int& Y, short& cont, char* Nombre) {
	dibujarTrigrama(Nombre[cont - 3], Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void Letras3J_I_L(int& X, int& Y, short& cont, char* Nombre) {
	dibujarTrigrama(Nombre[cont - 3], Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void Letras3J_M_P(int& X, int& Y, short& cont, char* Nombre) {
	dibujarTrigrama(Nombre[cont - 3], Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void Letras3J_Q_T(int& X, int& Y, short& cont, char* Nombre) {
	dibujarTrigrama(Nombre[cont - 3], Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void Letras3J_U_X(int& X, int& Y, short& cont, char* Nombre) {
	dibujarTrigrama(Nombre[cont - 3], Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void Letras3J_Y_Z(int& X, int& Y, short& cont, char* Nombre) {
	dibujarTrigrama(Nombre[cont - 3], Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void Letras3BJ_A_D(int& X, int& Y, short& cont, char* Nombre) {
	dibujarTrigrama(Nombre[cont - 3], Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void Letras3BJ_E_H(int& X, int& Y, short& cont, char* Nombre) {
	dibujarTrigrama(Nombre[cont - 3], Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void Letras3BJ_I_L(int& X, int& Y, short& cont, char* Nombre) {
	dibujarTrigrama(Nombre[cont - 3], Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void Letras3BJ_M_P(int& X, int& Y, short& cont, char* Nombre) {
	dibujarTrigrama(Nombre[cont - 3], Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void Letras3BJ_Q_T(int& X, int& Y, short& cont, char* Nombre) {
	dibujarTrigrama(Nombre[cont - 3], Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void Letras3BJ_U_X(int& X, int& Y, short& cont, char* Nombre) {
	dibujarTrigrama(Nombre[cont - 3], Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void Letras3BJ_Y_Z(int& X, int& Y, short& cont, char* Nombre) {
	dibujarTrigrama(Nombre[cont - 3], Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void Letras3CJ_A_D(int& X, int& Y, short& cont, char* Nombre) {
	dibujarTrigrama(Nombre[cont - 3], Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void Letras3CJ_E_H(int& X, int& Y, short& cont, char* Nombre) {
	dibujarTrigrama(Nombre[cont - 3], Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void Letras3CJ_I_L(int& X, int& Y, short& cont, char* Nombre) {
	dibujarTrigrama(Nombre[cont - 3], Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void Letras3CJ_M_P(int& X, int& Y, short& cont, char* Nombre) {
	dibujarTrigrama(Nombre[cont - 3], Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void Letras3CJ_Q_T(int& X, int& Y, short& cont, char* Nombre) {
	dibujarTrigrama(Nombre[cont - 3], Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void Letras3CJ_U_X(int& X, int& Y, short& cont, char* Nombre) {
	dibujarTrigrama(Nombre[cont - 3], Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void Letras3CJ_Y_Z(int& X, int& Y, short& cont, char* Nombre) {
	dibujarTrigrama(Nombre[cont - 3], Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}
void LetrA(int& X, int& Y, short& cont, char* Nombre) {
	dibujarTrigrama(Nombre[cont - 3], Nombre[cont - 2], Nombre[cont - 1], cont, X, Y);
}

void LetrO(int& X, int& Y, short& cont, char* Nombre) {
	int o = (cont * 2) + 1;
	int bJ = 103 - o;
	int bU = 103 - (cont - 1);
	int bA = 103 + (cont - 1);
	int bN = 103 + o;
	if (letraEnBBox(bJ, X, Y)) dibujarGlifoEn(Nombre[cont - 4], bJ, X, Y);
	else if (letraEnBBox(bU, X, Y)) dibujarGlifoEn(Nombre[cont - 3], bU, X, Y);
	else if (letraEnBBox(bA, X, Y)) dibujarGlifoEn(Nombre[cont - 2], bA, X, Y);
	else if (letraEnBBox(bN, X, Y)) dibujarGlifoEn(Nombre[cont - 1], bN, X, Y);
	else cout << " ";
}

void Opciones(int& X, int& Y, short& contL, short* ColorFJ) {
	if (contL == 0) {
		if (X == 102 && Y == 15) {
			cout << "G";
		}
		else
			if (X == 103 && Y == 15) {
				cout << "a";
			}
			else
				if (X == 104 && Y == 15) {
					cout << "n";
				}
				else
					if (X == 105 && Y == 15) {
						cout << "a";
					}
					else
						if (X == 106 && Y == 15) {
							cout << "d";
						}
						else
							if (X == 107 && Y == 15) {
								cout << "o";
							}
							else
								if (X == 108 && Y == 15) {
									cout << "r";
								}
								else
									if ((X == 118 || X == 119 || X == 120 || X == 121 || X == 122) && (Y == 44 || Y == 45 || Y == 46)) {
										color(hConsole, ColorFJ[0]);
										if ((X == 118) && Y == 44) {
											cout << " ";
										}
										else
											if ((X == 118) && Y == 45) {
												cout << char(220);
											}
											else
												if ((X == 118) && Y == 46) {
													cout << " ";
												}
												else
													if ((X == 119 || X == 120 || X == 121) && Y == 44) {
														cout << " ";
													}
													else
														if ((X == 119 || X == 120 || X == 121) && Y == 45) {
															cout << " ";
														}
														else
															if ((X == 119 || X == 120 || X == 121) && Y == 46) {
																cout << char(223);
															}
															else
																if (X == 122 && Y == 44) {
																	cout << char(219);
																}
																else
																	if (X == 122 && Y == 45) {
																		cout << char(219);
																	}
																	else
																		if (X == 122 && Y == 46) {
																			cout << " ";
																		}
									}
									else
										if ((X == 124 || X == 125 || X == 126 || X == 127 || X == 128) && (Y == 44 || Y == 45 || Y == 46)) {
											color(hConsole, ColorFJ[0]);
											if ((X == 124 || X == 127 || X == 128) && Y == 44) {
												cout << " ";
											}
											else
												if ((X == 124 || X == 127 || X == 128) && Y == 45) {
													cout << " ";
												}
												else
													if ((X == 124 || X == 127 || X == 128) && Y == 46) {
														cout << " ";
													}
													else
														if (X == 125 && Y == 44) {
															cout << char(223);
														}
														else
															if (X == 125 && Y == 45) {
																cout << " ";
															}
															else
																if (X == 125 && Y == 46) {
																	cout << " ";
																}
																else
																	if (X == 126 && Y == 44) {
																		cout << char(219);
																	}
																	else
																		if (X == 126 && Y == 45) {
																			cout << char(219);
																		}
																		else
																			if (X == 126 && Y == 46) {
																				cout << char(223);
																			}
										}
										else
											cout << " ";
	}
	else
		if (contL == 1) {
			if (X == 102 && Y == 15) {
				cout << "G";
			}
			else
				if (X == 103 && Y == 15) {
					cout << "a";
				}
				else
					if (X == 104 && Y == 15) {
						cout << "n";
					}
					else
						if (X == 105 && Y == 15) {
							cout << "a";
						}
						else
							if (X == 106 && Y == 15) {
								cout << "d";
							}
							else
								if (X == 107 && Y == 15) {
									cout << "o";
								}
								else
									if (X == 108 && Y == 15) {
										cout << "r";
									}
									else
										if ((X == 118 || X == 119 || X == 120 || X == 121 || X == 122) && (Y == 44 || Y == 45 || Y == 46)) {
											color(hConsole, ColorFJ[1]);
											if ((X == 118) && Y == 44) {
												cout << " ";
											}
											else
												if ((X == 118) && Y == 45) {
													cout << char(220);
												}
												else
													if ((X == 118) && Y == 46) {
														cout << " ";
													}
													else
														if ((X == 119 || X == 120 || X == 121) && Y == 44) {
															cout << " ";
														}
														else
															if ((X == 119 || X == 120 || X == 121) && Y == 45) {
																cout << " ";
															}
															else
																if ((X == 119 || X == 120 || X == 121) && Y == 46) {
																	cout << char(223);
																}
																else
																	if (X == 122 && Y == 44) {
																		cout << char(219);
																	}
																	else
																		if (X == 122 && Y == 45) {
																			cout << char(219);
																		}
																		else
																			if (X == 122 && Y == 46) {
																				cout << " ";
																			}
										}
										else
											if ((X == 124 || X == 125 || X == 126 || X == 127 || X == 128) && (Y == 44 || Y == 45 || Y == 46)) {
												color(hConsole, ColorFJ[1]);
												if (X == 124 && (Y == 44)) {
													cout << char(220);
												}
												else
													if (X == 124 && Y == 45) {
														cout << " ";
													}
													else
														if (X == 124 && Y == 46) {
															cout << char(223);
														}
														else
															if (X == 125 && (Y == 44)) {
																cout << char(223);
															}
															else
																if (X == 125 && Y == 45) {
																	cout << " ";
																}
																else
																	if (X == 125 && Y == 46) {
																		cout << char(223);
																	}
																	else
																		if (X == 126 && (Y == 44)) {
																			cout << char(223);
																		}
																		else
																			if (X == 126 && Y == 45) {
																				cout << char(220);
																			}
																			else
																				if (X == 126 && Y == 46) {
																					cout << char(223);
																				}
																				else
																					if (X == 127 && (Y == 44)) {
																						cout << char(223);
																					}
																					else
																						if (X == 127 && Y == 45) {
																							cout << char(223);
																						}
																						else
																							if (X == 127 && Y == 46) {
																								cout << char(223);
																							}
																							else
																								if (X == 128 && (Y == 44)) {
																									cout << char(220);
																								}
																								else
																									if (X == 128 && Y == 45) {
																										cout << " ";
																									}
																									else
																										if (X == 128 && Y == 46) {
																											cout << char(223);
																										}
											}
											else
												cout << " ";
		}
		else
			cout << " ";
}

void Instrucciones(int& X, int& Y) {
	system("cls");
	cout << "Fase 1: Colocaci" << char(162) << "n de las fichas" << endl << "En la primera fase, los jugadores colocan todas sus fichas sobre el tablero por turnos." << endl << "Si durante la fase de colocaci" << char(162) << "n un jugador consigue formar un tr" << char(161) << "o de fichas, podr" << char(160) << " eliminar una ficha ya colocada del oponente siempre que " << char(130) << "sta no est" << char(130) << " formando parte de un tr" << char(161) << "o." << endl << endl;
	cout << "Fase 2: Realizaci" << char(162) << "n de movimientos" << endl << "Una vez colocadas todas las fichas sobre el tablero, sin contar las eliminadas en la fase inicial, se pasar" << char(160) << " a la siguiente fase del juego." << endl << "En esta fase los jugadores mover" << char(160) << "n sus fichas, siempre por turnos sucesivos, siguiendo las l" << char(161) << "neas marcadas sobre el tablero." << endl << "En esta fase, al igual que en la fase inicial, si un jugador consigue formar un tr" << char(161) << "o de fichas podr" << char(160) << " eliminar una ficha del oponente siempre que " << char(130) << "sta no est" << char(130) << " formando parte de un tr" << char(161) << "o." << endl << "Esta fase concluye cuando uno de los dos jugadores tenga " << char(163) << "nicamente 3 fichas sobre el tablero o no pueda mover ninguna de sus fichas, por estar todas las casillas colindantes a las posiciones ocupadas por sus fichas ocupadas por una ficha suya o del rival." << endl << endl;
	cout << "Fase 3: Fase Final. Movimientos libres" << endl << "La fase final comienza cuando uno de los dos jugadores consigue que su oponente s" << char(162) << "lo tenga 3 fichas sobre el tablero." << endl << "En esta " << char(163) << "ltima fase, el jugador que s" << char(162) << "lo tenga 3 fichas sobre el tablero, o los dos jugadores en caso de que ambos tengan 3 fichas, podr" << char(160) << " moverse sin estar obligado a seguir las l" << char(161) << "neas dibujadas sobre el tablero" << endl << "Al igual que en las fases anteriores, si un jugador consigue formar un tr" << char(161) << "o de fichas podr" << char(160) << " eliminar una ficha del oponente siempre que " << char(130) << "sta no est" << char(130) << " formando parte de un tr" << char(161) << "o." << endl << "Esta fase, y con ella la partida, terminar" << char(160) << " cuando uno de los jugadores obtenga la victoria consiguiendo dejar a su oponente con s" << char(162) << "lo 2 fichas sobre el tablero";
	getch();
}

void Salir(int& X, int& Y, int* YFlch, int* XFlch) {
	if ((X == 51 || X == 160) && (Y == 10 || Y == 11 || Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32 || Y == 33 || Y == 34 || Y == 35 || Y == 36 || Y == 37 || Y == 38 || Y == 39 || Y == 40)) {
		color(hConsole, 15);
		cout << char(219);
	}
	else
		if (Y == 40 && (X == 52 || X == 53 || X == 54 || X == 55 || X == 56 || X == 57 || X == 58 || X == 59 || X == 60 || X == 61 || X == 62 || X == 63 || X == 64 || X == 65 || X == 66 || X == 67 || X == 68 || X == 69 || X == 70 || X == 71 || X == 72 || X == 73 || X == 74 || X == 75 || X == 76 || X == 77 || X == 78 || X == 79 || X == 80 || X == 81 || X == 82 || X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130 || X == 131 || X == 132 || X == 133 || X == 134 || X == 135 || X == 136 || X == 137 || X == 138 || X == 139 || X == 140 || X == 141 || X == 142 || X == 143 || X == 144 || X == 145 || X == 146 || X == 147 || X == 148 || X == 149 || X == 150 || X == 151 || X == 152 || X == 153 || X == 154 || X == 155 || X == 156 || X == 157 || X == 158 || X == 159)) {
			color(hConsole, 15);
			cout << char(220);
		}
		else
			if (Y == 10 && (X == 52 || X == 53 || X == 54 || X == 55 || X == 56 || X == 57 || X == 58 || X == 59 || X == 60 || X == 61 || X == 62 || X == 63 || X == 64 || X == 65 || X == 66 || X == 67 || X == 68 || X == 69 || X == 70 || X == 71 || X == 72 || X == 73 || X == 74 || X == 75 || X == 76 || X == 77 || X == 78 || X == 79 || X == 80 || X == 81 || X == 82 || X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130 || X == 131 || X == 132 || X == 133 || X == 134 || X == 135 || X == 136 || X == 137 || X == 138 || X == 139 || X == 140 || X == 141 || X == 142 || X == 143 || X == 144 || X == 145 || X == 146 || X == 147 || X == 148 || X == 149 || X == 150 || X == 151 || X == 152 || X == 153 || X == 154 || X == 155 || X == 156 || X == 157 || X == 158 || X == 159)) {
				color(hConsole, 15);
				cout << char(223);
			}
			else
				if (X == 88 && Y == 19) {  //  S
					color(hConsole, 15);
					cout << char(220);
				}
				else
					if (X == 88 && Y == 20) {
						cout << " ";
					}
					else
						if (X == 88 && Y == 21) {
							color(hConsole, 15);
							cout << char(223);
						}
						else
							if ((X == 89 || X == 90 || X == 91) && Y == 19) {
								color(hConsole, 15);
								cout << char(223);
							}
							else
								if ((X == 89 || X == 90 || X == 91) && Y == 20) {
									color(hConsole, 15);
									cout << char(223);
								}
								else
									if ((X == 89 || X == 90 || X == 91) && Y == 21) {
										color(hConsole, 15);
										cout << char(223);
									}
									else
										if (X == 92 && Y == 19) {
											color(hConsole, 15);
											cout << char(223);
										}
										else
											if (X == 92 && Y == 20) {
												cout << char(220);
											}
											else
												if (X == 92 && Y == 21) {
													cout << " ";
												}
												else  //  Fin S
													if ((X == 94 || X == 98) && Y == 19) {  //  A
														cout << char(220);
													}
													else
														if ((X == 94 || X == 98) && Y == 20) {
															cout << char(219);
														}
														else
															if ((X == 94 || X == 98) && Y == 21) {
																cout << char(223);
															}
															else
																if ((X == 95 || X == 96 || X == 97) && Y == 19) {
																	cout << char(223);
																}
																else
																	if ((X == 95 || X == 96 || X == 97) && Y == 20) {
																		cout << char(223);
																	}
																	else
																		if ((X == 95 || X == 96 || X == 97) && Y == 21) {
																			cout << " ";
																		}
																		else  // Fin A
																			if (X == 100 && Y == 19) {  //  L
																				cout << char(219);
																			}
																			else
																				if (X == 100 && Y == 20) {
																					cout << char(219);
																				}
																				else
																					if (X == 100 && Y == 21) {
																						cout << char(223);
																					}
																					else
																						if ((X == 101 || X == 102 || X == 103 || X == 104) && Y == 19) {
																							cout << " ";
																						}
																						else
																							if ((X == 101 || X == 102 || X == 103 || X == 104) && Y == 20) {
																								cout << " ";
																							}
																							else
																								if ((X == 101 || X == 102 || X == 103 || X == 104) && Y == 21) {
																									cout << char(223);
																								}
																								else  //  Fin L
																									if ((X == 106 || X == 107 || X == 109 || X == 110) && Y == 19) {  //  I
																										cout << char(223);
																									}
																									else
																										if ((X == 106 || X == 107 || X == 109 || X == 110) && Y == 20) {
																											cout << " ";
																										}
																										else
																											if ((X == 106 || X == 107 || X == 109 || X == 110) && Y == 21) {
																												cout << char(223);
																											}
																											else
																												if (X == 108 && Y == 19) {
																													cout << char(219);
																												}
																												else
																													if (X == 108 && Y == 20) {
																														cout << char(219);
																													}
																													else
																														if (X == 108 && Y == 21) {
																															cout << char(223);
																														}
																														else  //  Fin I
																															if (X == 112 && Y == 19) {  //  R	
																																cout << char(219);
																															}
																															else
																																if (X == 112 && Y == 20) {
																																	cout << char(219);
																																}
																																else
																																	if (X == 112 && Y == 21) {
																																		cout << char(223);
																																	}
																																	else
																																		if ((X == 113 || X == 114) && Y == 19) {
																																			cout << char(223);
																																		}
																																		else
																																			if ((X == 113 || X == 114) && Y == 20) {
																																				cout << char(223);
																																			}
																																			else
																																				if ((X == 113 || X == 114) && Y == 21) {
																																					cout << " ";
																																				}
																																				else
																																					if (X == 115 && Y == 19) {
																																						cout << char(223);
																																					}
																																					else
																																						if (X == 115 && Y == 20) {
																																							cout << char(219);
																																						}
																																						else
																																							if (X == 115 && Y == 21) {
																																								cout << " ";
																																							}
																																							else
																																								if (X == 116 && Y == 19) {
																																									cout << char(220);
																																								}
																																								else
																																									if (X == 116 && Y == 20) {
																																										cout << char(223);
																																									}
																																									else
																																										if (X == 116 && Y == 21) {
																																											cout << char(223);
																																										}
																																										else	//  Fin R
																																											if (X == 119 && Y == 19) {  //  ?
																																												cout << char(220);
																																											}
																																											else
																																												if (X == 119 && Y == 20) {
																																													cout << " ";
																																												}
																																												else
																																													if (X == 119 && Y == 21) {
																																														cout << " ";
																																													}
																																													else
																																														if (X == 120 && Y == 19) {
																																															cout << char(223);
																																														}
																																														else
																																															if (X == 120 && Y == 20) {
																																																cout << " ";
																																															}
																																															else
																																																if (X == 120 && Y == 21) {
																																																	cout << " ";
																																																}
																																																else
																																																	if (X == 121 && Y == 19) {
																																																		cout << char(223);
																																																	}
																																																	else
																																																		if (X == 121 && Y == 20) {
																																																			cout << char(223);
																																																		}
																																																		else
																																																			if (X == 121 && Y == 21) {
																																																				cout << char(223);
																																																			}
																																																			else
																																																				if (X == 122 && Y == 19) {
																																																					cout << char(223);
																																																				}
																																																				else
																																																					if (X == 122 && Y == 20) {
																																																						cout << char(223);
																																																					}
																																																					else
																																																						if (X == 122 && Y == 21) {
																																																							cout << " ";
																																																						}
																																																						else
																																																							if (X == 123 && Y == 19) {
																																																								cout << char(220);
																																																							}
																																																							else
																																																								if (X == 123 && Y == 20) {
																																																									cout << " ";
																																																								}
																																																								else
																																																									if (X == 123 && Y == 21) {
																																																										cout << " ";
																																																									}
																																																									else  //  Fin ?
																																																										if (X == 77 && Y == 32) {  //  S
																																																											if (*XFlch == 69 && *YFlch == 32) {
																																																												color(hConsole, 14);
																																																												cout << char(220);
																																																											}
																																																											else
																																																												if (*XFlch == 116 && *YFlch == 32) {
																																																													color(hConsole, 7);
																																																													cout << char(220);
																																																												}
																																																										}
																																																										else
																																																											if (X == 77 && Y == 33) {
																																																												cout << " ";
																																																											}
																																																											else
																																																												if (X == 77 && Y == 34) {
																																																													if (*XFlch == 69 && *YFlch == 32) {
																																																														color(hConsole, 14);
																																																														cout << char(223);
																																																													}
																																																													else
																																																														if (*XFlch == 116 && *YFlch == 32) {
																																																															color(hConsole, 7);
																																																															cout << char(223);
																																																														}
																																																												}
																																																												else
																																																													if ((X == 78 || X == 79 || X == 80) && Y == 32) {
																																																														if (*XFlch == 69 && *YFlch == 32) {
																																																															color(hConsole, 14);
																																																															cout << char(223);
																																																														}
																																																														else
																																																															if (*XFlch == 116 && *YFlch == 32) {
																																																																color(hConsole, 7);
																																																																cout << char(223);
																																																															}
																																																													}
																																																													else
																																																														if ((X == 78 || X == 79 || X == 80) && Y == 33) {
																																																															if (*XFlch == 69 && *YFlch == 32) {
																																																																color(hConsole, 14);
																																																																cout << char(223);
																																																															}
																																																															else
																																																																if (*XFlch == 116 && *YFlch == 32) {
																																																																	color(hConsole, 7);
																																																																	cout << char(223);
																																																																}
																																																														}
																																																														else
																																																															if ((X == 78 || X == 79 || X == 80) && Y == 34) {
																																																																if (*XFlch == 69 && *YFlch == 32) {
																																																																	color(hConsole, 14);
																																																																	cout << char(223);
																																																																}
																																																																else
																																																																	if (*XFlch == 116 && *YFlch == 32) {
																																																																		color(hConsole, 7);
																																																																		cout << char(223);
																																																																	}
																																																															}
																																																															else
																																																																if (X == 81 && Y == 32) {
																																																																	if (*XFlch == 69 && *YFlch == 32) {
																																																																		color(hConsole, 14);
																																																																		cout << char(223);
																																																																	}
																																																																	else
																																																																		if (*XFlch == 116 && *YFlch == 32) {
																																																																			color(hConsole, 7);
																																																																			cout << char(223);
																																																																		}
																																																																}
																																																																else
																																																																	if (X == 81 && Y == 33) {
																																																																		if (*XFlch == 69 && *YFlch == 32) {
																																																																			color(hConsole, 14);
																																																																			cout << char(220);
																																																																		}
																																																																		else
																																																																			if (*XFlch == 116 && *YFlch == 32) {
																																																																				color(hConsole, 7);
																																																																				cout << char(220);
																																																																			}
																																																																	}
																																																																	else
																																																																		if (X == 81 && Y == 34) {
																																																																			cout << " ";
																																																																		}
																																																																		else  //  Fin S
																																																																			if ((X == 83 || X == 84 || X == 86 || X == 87) && Y == 32) {  //  I
																																																																				if (*XFlch == 69 && *YFlch == 32) {
																																																																					color(hConsole, 14);
																																																																					cout << char(223);
																																																																				}
																																																																				else
																																																																					if (*XFlch == 116 && *YFlch == 32) {
																																																																						color(hConsole, 7);
																																																																						cout << char(223);
																																																																					}
																																																																			}
																																																																			else
																																																																				if ((X == 83 || X == 84 || X == 86 || X == 87) && Y == 33) {
																																																																					cout << " ";
																																																																				}
																																																																				else
																																																																					if ((X == 83 || X == 84 || X == 86 || X == 87) && Y == 34) {
																																																																						if (*XFlch == 69 && *YFlch == 32) {
																																																																							color(hConsole, 14);
																																																																							cout << char(223);
																																																																						}
																																																																						else
																																																																							if (*XFlch == 116 && *YFlch == 32) {
																																																																								color(hConsole, 7);
																																																																								cout << char(223);
																																																																							}
																																																																					}
																																																																					else
																																																																						if (X == 85 && Y == 32) {
																																																																							if (*XFlch == 69 && *YFlch == 32) {
																																																																								color(hConsole, 14);
																																																																								cout << char(219);
																																																																							}
																																																																							else
																																																																								if (*XFlch == 116 && *YFlch == 32) {
																																																																									color(hConsole, 7);
																																																																									cout << char(219);
																																																																								}
																																																																						}
																																																																						else
																																																																							if (X == 85 && Y == 33) {
																																																																								if (*XFlch == 69 && *YFlch == 32) {
																																																																									color(hConsole, 14);
																																																																									cout << char(219);
																																																																								}
																																																																								else
																																																																									if (*XFlch == 116 && *YFlch == 32) {
																																																																										color(hConsole, 7);
																																																																										cout << char(219);
																																																																									}
																																																																							}
																																																																							else
																																																																								if (X == 85 && Y == 34) {
																																																																									if (*XFlch == 69 && *YFlch == 32) {
																																																																										color(hConsole, 14);
																																																																										cout << char(223);
																																																																									}
																																																																									else
																																																																										if (*XFlch == 116 && *YFlch == 32) {
																																																																											color(hConsole, 7);
																																																																											cout << char(223);
																																																																										}
																																																																								}
																																																																								else  //  Fin I
																																																																									if ((X == 124 || X == 128) && Y == 32) {  //  N	
																																																																										if (*XFlch == 116 && *YFlch == 32) {
																																																																											color(hConsole, 14);
																																																																											cout << char(219);
																																																																										}
																																																																										else
																																																																											if (*XFlch == 69 && *YFlch == 32) {
																																																																												color(hConsole, 7);
																																																																												cout << char(219);
																																																																											}
																																																																									}
																																																																									else
																																																																										if ((X == 124 || X == 128) && Y == 33) {
																																																																											if (*XFlch == 116 && *YFlch == 32) {
																																																																												color(hConsole, 14);
																																																																												cout << char(219);
																																																																											}
																																																																											else
																																																																												if (*XFlch == 69 && *YFlch == 32) {
																																																																													color(hConsole, 7);
																																																																													cout << char(219);
																																																																												}
																																																																										}
																																																																										else
																																																																											if ((X == 124 || X == 128) && Y == 34) {
																																																																												if (*XFlch == 116 && *YFlch == 32) {
																																																																													color(hConsole, 14);
																																																																													cout << char(223);
																																																																												}
																																																																												else
																																																																													if (*XFlch == 69 && *YFlch == 32) {
																																																																														color(hConsole, 7);
																																																																														cout << char(223);
																																																																													}
																																																																											}
																																																																											else
																																																																												if (X == 125 && Y == 32) {
																																																																													if (*XFlch == 116 && *YFlch == 32) {
																																																																														color(hConsole, 14);
																																																																														cout << char(220);
																																																																													}
																																																																													else
																																																																														if (*XFlch == 69 && *YFlch == 32) {
																																																																															color(hConsole, 7);
																																																																															cout << char(220);
																																																																														}
																																																																												}
																																																																												else
																																																																													if (X == 125 && Y == 33) {
																																																																														cout << " ";
																																																																													}
																																																																													else
																																																																														if (X == 125 && Y == 34) {
																																																																															cout << " ";
																																																																														}
																																																																														else
																																																																															if (X == 126 && Y == 32) {
																																																																																cout << " ";
																																																																															}
																																																																															else
																																																																																if (X == 126 && Y == 33) {
																																																																																	if (*XFlch == 116 && *YFlch == 32) {
																																																																																		color(hConsole, 14);
																																																																																		cout << char(223);
																																																																																	}
																																																																																	else
																																																																																		if (*XFlch == 69 && *YFlch == 32) {
																																																																																			color(hConsole, 7);
																																																																																			cout << char(223);
																																																																																		}
																																																																																}
																																																																																else
																																																																																	if (X == 126 && Y == 34) {
																																																																																		cout << " ";
																																																																																	}
																																																																																	else
																																																																																		if (X == 127 && Y == 32) {
																																																																																			cout << " ";
																																																																																		}
																																																																																		else
																																																																																			if (X == 127 && Y == 33) {
																																																																																				if (*XFlch == 116 && *YFlch == 32) {
																																																																																					color(hConsole, 14);
																																																																																					cout << char(220);
																																																																																				}
																																																																																				else
																																																																																					if (*XFlch == 69 && *YFlch == 32) {
																																																																																						color(hConsole, 7);
																																																																																						cout << char(220);
																																																																																					}
																																																																																			}
																																																																																			else
																																																																																				if (X == 127 && Y == 34) {
																																																																																					cout << " ";
																																																																																				}
																																																																																				else  //  Fin N
																																																																																					if ((X == 130 || X == 134) && Y == 32) {  //  O
																																																																																						if (*XFlch == 116 && *YFlch == 32) {
																																																																																							color(hConsole, 14);
																																																																																							cout << char(220);
																																																																																						}
																																																																																						else
																																																																																							if (*XFlch == 69 && *YFlch == 32) {
																																																																																								color(hConsole, 7);
																																																																																								cout << char(220);
																																																																																							}
																																																																																					}
																																																																																					else
																																																																																						if ((X == 130 || X == 134) && Y == 33) {
																																																																																							if (*XFlch == 116 && *YFlch == 32) {
																																																																																								color(hConsole, 14);
																																																																																								cout << char(219);
																																																																																							}
																																																																																							else
																																																																																								if (*XFlch == 69 && *YFlch == 32) {
																																																																																									color(hConsole, 7);
																																																																																									cout << char(219);
																																																																																								}
																																																																																						}
																																																																																						else
																																																																																							if ((X == 130 || X == 134) && Y == 34) {
																																																																																								cout << " ";
																																																																																							}
																																																																																							else
																																																																																								if ((X == 131 || X == 132 || X == 133) && Y == 32) {
																																																																																									if (*XFlch == 116 && *YFlch == 32) {
																																																																																										color(hConsole, 14);
																																																																																										cout << char(223);
																																																																																									}
																																																																																									else
																																																																																										if (*XFlch == 69 && *YFlch == 32) {
																																																																																											color(hConsole, 7);
																																																																																											cout << char(223);
																																																																																										}
																																																																																								}
																																																																																								else
																																																																																									if ((X == 131 || X == 132 || X == 133) && Y == 33) {
																																																																																										cout << " ";
																																																																																									}
																																																																																									else
																																																																																										if ((X == 131 || X == 132 || X == 133) && Y == 34) {
																																																																																											if (*XFlch == 116 && *YFlch == 32) {
																																																																																												color(hConsole, 14);
																																																																																												cout << char(223);
																																																																																											}
																																																																																											else
																																																																																												if (*XFlch == 69 && *YFlch == 32) {
																																																																																													color(hConsole, 7);
																																																																																													cout << char(223);
																																																																																												}
																																																																																										}
																																																																																										else  //  Fin O
																																																																																											if (*XFlch == 69 && *YFlch == 32) {
																																																																																												if (X == 69 && (Y == 32 || Y == 34) || X == 68 && (Y == 32 || Y == 33 || Y == 34) || X == 95 && (Y == 32 || Y == 34) || X == 96 && (Y == 32 || Y == 33 || Y == 34)) {
																																																																																													cout << "";
																																																																																												}
																																																																																												else
																																																																																													if (X == 116 && (Y == 32 || Y == 34) || X == 115 && (Y == 32 || Y == 33 || Y == 34) || X == 142 && (Y == 32 || Y == 34) || X == 143 && (Y == 32 || Y == 33 || Y == 34)) {
																																																																																														cout << " ";
																																																																																													}
																																																																																													else
																																																																																														cout << " ";
																																																																																											}
																																																																																											else
																																																																																												if (*XFlch == 116 && *YFlch == 32) {
																																																																																													if (X == 116 && (Y == 32 || Y == 34) || X == 115 && (Y == 32 || Y == 33 || Y == 34) || X == 142 && (Y == 32 || Y == 34) || X == 143 && (Y == 32 || Y == 33 || Y == 34)) {
																																																																																														cout << "";
																																																																																													}
																																																																																													else
																																																																																														if (X == 69 && (Y == 32 || Y == 34) || X == 68 && (Y == 32 || Y == 33 || Y == 34) || X == 95 && (Y == 32 || Y == 34) || X == 96 && (Y == 32 || Y == 33 || Y == 34)) {
																																																																																															cout << " ";
																																																																																														}
																																																																																														else
																																																																																															cout << " ";
																																																																																												}
																																																																																												else
																																																																																													cout << "";
}

void Cr�ditos(int& X, int& Y) {
	system("cls");
	cout << "Leonardo Leoncio Bravo Ricapa (u20211c688) Estudiante" << endl;
	cout << "Felipe Fabrizio Baumann Serna (u202111989) Estudiante" << endl;
	cout << "Michael Wilmer Vasquez Leon (u201923943) Estudiante" << endl;
	cout << "Juan Orlando Salazar Campos Profesor" << endl;
	getch();
}

void Dados(int& X, int& Y, short* Turno, short* ColorD, short& contL, short& R) {

	if (contL == 0) {
		if (R == 1) {
			color(hConsole, ColorD[0]);
			if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && (Y == 33) || (X == 81 || X == 132) && (Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32) || (X == 82 || X == 131) && (Y == 11 || Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32)) {
				cout << char(219);
			}
			else
				if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && (Y == 11) || (X == 105 || X == 109) && Y == 22 || (X == 82 || X == 131) && (Y == 11 || Y == 33)) {
					cout << char(223);
				}
				else
					if ((X == 106 || X == 107 || X == 108) && (Y == 21 || Y == 22) || (X == 105 || X == 109) && Y == 21) {
						cout << char(219);
					}
					else
						if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && Y == 10 || (X == 81 || X == 132) && Y == 11 || (X == 106 || X == 107 || X == 108) && Y == 20) {
							cout << char(220);
						}
						else
							if (X == 98 && (Y == 39 || Y == 40)) {
								cout << char(219);
							}
							else
								if ((X == 98 || X == 99 || X == 100 || X == 101 || X == 102) && Y == 41) {
									cout << char(223);
								}
								else
									if (X == 104 && (Y == 39 || Y == 40)) {
										cout << char(219);
									}
									else
										if ((X == 104 || X == 105 || X == 106 || X == 107 || X == 108) && (Y == 41 || Y == 39)) {
											cout << char(223);
										}
										else
											if ((X == 105 || X == 106 || X == 107) && Y == 40) {
												cout << char(223);
											}
											else
												if ((X == 110 || X == 114) && Y == 39) {
													cout << char(220);
												}
												else
													if ((X == 110 || X == 114) && Y == 40) {
														cout << char(219);
													}
													else
														if ((X == 111 || X == 112 || X == 113) && (Y == 39 || Y == 41)) {
															cout << char(223);
														}
														else
															cout << " ";

		}
		else
			if (R == 2) {
				color(hConsole, ColorD[1]);
				if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && (Y == 33) || (X == 81 || X == 132) && (Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32) || (X == 82 || X == 131) && (Y == 11 || Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32)) {
					cout << char(219);
				}
				else
					if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && (Y == 11) || (X == 82 || X == 131) && (Y == 11 || Y == 33)) {
						cout << char(223);
					}
					else
						if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && Y == 10 || (X == 81 || X == 132) && Y == 11) {
							cout << char(220);
						}
						else
							if ((X == 92 || X == 93 || X == 94) && (Y == 16 || Y == 15) || (X == 91 || X == 95) && Y == 15) {
								cout << char(219);
							}
							else
								if ((X == 92 || X == 93 || X == 94) && Y == 14) {
									cout << char(220);
								}
								else
									if ((X == 91 || X == 95) && Y == 16) {
										cout << char(223);
									}
									else
										if ((X == 119 || X == 120 || X == 121) && (Y == 27 || Y == 28) || (X == 118 || X == 122) && Y == 27) {
											cout << char(219);
										}
										else
											if ((X == 119 || X == 120 || X == 121) && Y == 26) {
												cout << char(220);
											}
											else
												if ((X == 118 || X == 122) && Y == 28) {
													cout << char(223);
												}
												else
													if (X == 98 && (Y == 39 || Y == 40)) {
														cout << char(219);
													}
													else
														if ((X == 98 || X == 99 || X == 100 || X == 101 || X == 102) && Y == 41) {
															cout << char(223);
														}
														else
															if (X == 104 && (Y == 39 || Y == 40)) {
																cout << char(219);
															}
															else
																if ((X == 104 || X == 105 || X == 106 || X == 107 || X == 108) && (Y == 41 || Y == 39)) {
																	cout << char(223);
																}
																else
																	if ((X == 105 || X == 106 || X == 107) && Y == 40) {
																		cout << char(223);
																	}
																	else
																		if ((X == 110 || X == 114) && Y == 39) {
																			cout << char(220);
																		}
																		else
																			if ((X == 110 || X == 114) && Y == 40) {
																				cout << char(219);
																			}
																			else
																				if ((X == 111 || X == 112 || X == 113) && (Y == 39 || Y == 41)) {
																					cout << char(223);
																				}
																				else
																					cout << " ";
			}
			else
				if (R == 3) {
					color(hConsole, ColorD[2]);
					if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && (Y == 33) || (X == 81 || X == 132) && (Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32) || (X == 82 || X == 131) && (Y == 11 || Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32)) {
						cout << char(219);
					}
					else
						if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && (Y == 11) || (X == 105 || X == 109) && Y == 22 || (X == 82 || X == 131) && (Y == 11 || Y == 33)) {
							cout << char(223);
						}
						else
							if ((X == 106 || X == 107 || X == 108) && (Y == 21 || Y == 22) || (X == 105 || X == 109) && Y == 21) {
								cout << char(219);
							}
							else
								if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && Y == 10 || (X == 81 || X == 132) && Y == 11 || (X == 106 || X == 107 || X == 108) && Y == 20) {
									cout << char(220);
								}
								else
									if ((X == 92 || X == 93 || X == 94) && (Y == 16 || Y == 15) || (X == 91 || X == 95) && Y == 15) {
										cout << char(219);
									}
									else
										if ((X == 92 || X == 93 || X == 94) && Y == 14) {
											cout << char(220);
										}
										else
											if ((X == 91 || X == 95) && Y == 16) {
												cout << char(223);
											}
											else
												if ((X == 119 || X == 120 || X == 121) && (Y == 27 || Y == 28) || (X == 118 || X == 122) && Y == 27) {
													cout << char(219);
												}
												else
													if ((X == 119 || X == 120 || X == 121) && Y == 26) {
														cout << char(220);
													}
													else
														if ((X == 118 || X == 122) && Y == 28) {
															cout << char(223);
														}
														else
															if (X == 98 && (Y == 39 || Y == 40)) {
																cout << char(219);
															}
															else
																if ((X == 98 || X == 99 || X == 100 || X == 101 || X == 102) && Y == 41) {
																	cout << char(223);
																}
																else
																	if (X == 104 && (Y == 39 || Y == 40)) {
																		cout << char(219);
																	}
																	else
																		if ((X == 104 || X == 105 || X == 106 || X == 107 || X == 108) && (Y == 41 || Y == 39)) {
																			cout << char(223);
																		}
																		else
																			if ((X == 105 || X == 106 || X == 107) && Y == 40) {
																				cout << char(223);
																			}
																			else
																				if ((X == 110 || X == 114) && Y == 39) {
																					cout << char(220);
																				}
																				else
																					if ((X == 110 || X == 114) && Y == 40) {
																						cout << char(219);
																					}
																					else
																						if ((X == 111 || X == 112 || X == 113) && (Y == 39 || Y == 41)) {
																							cout << char(223);
																						}
																						else
																							cout << " ";
				}
				else
					if (R == 4) {
						color(hConsole, ColorD[3]);
						if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && (Y == 33) || (X == 81 || X == 132) && (Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32) || (X == 82 || X == 131) && (Y == 11 || Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32)) {
							cout << char(219);
						}
						else
							if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && (Y == 11) || (X == 82 || X == 131) && (Y == 11 || Y == 33)) {
								cout << char(223);
							}
							else
								if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && Y == 10 || (X == 81 || X == 132) && Y == 11) {
									cout << char(220);
								}
								else
									if ((X == 92 || X == 93 || X == 94) && (Y == 16 || Y == 15) || (X == 91 || X == 95) && Y == 15) {
										cout << char(219);
									}
									else
										if ((X == 92 || X == 93 || X == 94) && Y == 14) {
											cout << char(220);
										}
										else
											if ((X == 91 || X == 95) && Y == 16) {
												cout << char(223);
											}
											else
												if ((X == 92 || X == 93 || X == 94) && (Y == 27 || Y == 28) || (X == 91 || X == 95) && Y == 27) {
													cout << char(219);
												}
												else
													if ((X == 92 || X == 93 || X == 94) && Y == 26) {
														cout << char(220);
													}
													else
														if ((X == 91 || X == 95) && Y == 28) {
															cout << char(223);
														}
														else
															if ((X == 119 || X == 120 || X == 121) && (Y == 27 || Y == 28) || (X == 118 || X == 122) && Y == 27) {
																cout << char(219);
															}
															else
																if ((X == 119 || X == 120 || X == 121) && Y == 26) {
																	cout << char(220);
																}
																else
																	if ((X == 118 || X == 122) && Y == 28) {
																		cout << char(223);
																	}
																	else
																		if ((X == 119 || X == 120 || X == 121) && (Y == 16 || Y == 15) || (X == 118 || X == 122) && Y == 15) {
																			cout << char(219);
																		}
																		else
																			if ((X == 119 || X == 120 || X == 121) && Y == 14) {
																				cout << char(220);
																			}
																			else
																				if ((X == 118 || X == 122) && Y == 16) {
																					cout << char(223);
																				}
																				else
																					if (X == 98 && (Y == 39 || Y == 40)) {
																						cout << char(219);
																					}
																					else
																						if ((X == 98 || X == 99 || X == 100 || X == 101 || X == 102) && Y == 41) {
																							cout << char(223);
																						}
																						else
																							if (X == 104 && (Y == 39 || Y == 40)) {
																								cout << char(219);
																							}
																							else
																								if ((X == 104 || X == 105 || X == 106 || X == 107 || X == 108) && (Y == 41 || Y == 39)) {
																									cout << char(223);
																								}
																								else
																									if ((X == 105 || X == 106 || X == 107) && Y == 40) {
																										cout << char(223);
																									}
																									else
																										if ((X == 110 || X == 114) && Y == 39) {
																											cout << char(220);
																										}
																										else
																											if ((X == 110 || X == 114) && Y == 40) {
																												cout << char(219);
																											}
																											else
																												if ((X == 111 || X == 112 || X == 113) && (Y == 39 || Y == 41)) {
																													cout << char(223);
																												}
																												else
																													cout << " ";
					}
					else
						if (R == 5) {
							color(hConsole, ColorD[4]);
							if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && (Y == 33) || (X == 81 || X == 132) && (Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32) || (X == 82 || X == 131) && (Y == 11 || Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32)) {
								cout << char(219);
							}
							else
								if ((X == 105 || X == 109) && Y == 22 || (X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && (Y == 11) || (X == 82 || X == 131) && (Y == 11 || Y == 33)) {
									cout << char(223);
								}
								else
									if ((X == 106 || X == 107 || X == 108) && Y == 20 || (X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && Y == 10 || (X == 81 || X == 132) && Y == 11) {
										cout << char(220);
									}
									else
										if ((X == 106 || X == 107 || X == 108) && (Y == 21 || Y == 22) || (X == 105 || X == 109) && Y == 21 || (X == 92 || X == 93 || X == 94) && (Y == 16 || Y == 15) || (X == 91 || X == 95) && Y == 15) {
											cout << char(219);
										}
										else
											if ((X == 92 || X == 93 || X == 94) && Y == 14) {
												cout << char(220);
											}
											else
												if ((X == 91 || X == 95) && Y == 16) {
													cout << char(223);
												}
												else
													if ((X == 92 || X == 93 || X == 94) && (Y == 27 || Y == 28) || (X == 91 || X == 95) && Y == 27) {
														cout << char(219);
													}
													else
														if ((X == 92 || X == 93 || X == 94) && Y == 26) {
															cout << char(220);
														}
														else
															if ((X == 91 || X == 95) && Y == 28) {
																cout << char(223);
															}
															else
																if ((X == 119 || X == 120 || X == 121) && (Y == 27 || Y == 28) || (X == 118 || X == 122) && Y == 27) {
																	cout << char(219);
																}
																else
																	if ((X == 119 || X == 120 || X == 121) && Y == 26) {
																		cout << char(220);
																	}
																	else
																		if ((X == 118 || X == 122) && Y == 28) {
																			cout << char(223);
																		}
																		else
																			if ((X == 119 || X == 120 || X == 121) && (Y == 16 || Y == 15) || (X == 118 || X == 122) && Y == 15) {
																				cout << char(219);
																			}
																			else
																				if ((X == 119 || X == 120 || X == 121) && Y == 14) {
																					cout << char(220);
																				}
																				else
																					if ((X == 118 || X == 122) && Y == 16) {
																						cout << char(223);
																					}
																					else
																						if (X == 98 && (Y == 39 || Y == 40)) {
																							cout << char(219);
																						}
																						else
																							if ((X == 98 || X == 99 || X == 100 || X == 101 || X == 102) && Y == 41) {
																								cout << char(223);
																							}
																							else
																								if (X == 104 && (Y == 39 || Y == 40)) {
																									cout << char(219);
																								}
																								else
																									if ((X == 104 || X == 105 || X == 106 || X == 107 || X == 108) && (Y == 41 || Y == 39)) {
																										cout << char(223);
																									}
																									else
																										if ((X == 105 || X == 106 || X == 107) && Y == 40) {
																											cout << char(223);
																										}
																										else
																											if ((X == 110 || X == 114) && Y == 39) {
																												cout << char(220);
																											}
																											else
																												if ((X == 110 || X == 114) && Y == 40) {
																													cout << char(219);
																												}
																												else
																													if ((X == 111 || X == 112 || X == 113) && (Y == 39 || Y == 41)) {
																														cout << char(223);
																													}
																													else
																														cout << " ";
						}
						else
							if (R == 6) {
								color(hConsole, ColorD[5]);
								if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && (Y == 33) || (X == 81 || X == 132) && (Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32) || (X == 82 || X == 131) && (Y == 11 || Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32)) {
									cout << char(219);
								}
								else
									if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && (Y == 11) || (X == 82 || X == 131) && (Y == 11 || Y == 33)) {
										cout << char(223);
									}
									else
										if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && Y == 10 || (X == 81 || X == 132) && Y == 11) {
											cout << char(220);
										}
										else
											if ((X == 92 || X == 93 || X == 94) && (Y == 16 || Y == 15) || (X == 91 || X == 95) && Y == 15) {
												cout << char(219);
											}
											else
												if ((X == 92 || X == 93 || X == 94) && Y == 14) {
													cout << char(220);
												}
												else
													if ((X == 91 || X == 95) && Y == 16) {
														cout << char(223);
													}
													else
														if ((X == 92 || X == 93 || X == 94) && (Y == 22 || Y == 21) || (X == 91 || X == 95) && Y == 21) {
															cout << char(219);
														}
														else
															if ((X == 92 || X == 93 || X == 94) && Y == 20) {
																cout << char(220);
															}
															else
																if ((X == 91 || X == 95) && Y == 22) {
																	cout << char(223);
																}
																else
																	if ((X == 92 || X == 93 || X == 94) && (Y == 27 || Y == 28) || (X == 91 || X == 95) && Y == 27) {
																		cout << char(219);
																	}
																	else
																		if ((X == 92 || X == 93 || X == 94) && Y == 26) {
																			cout << char(220);
																		}
																		else
																			if ((X == 91 || X == 95) && Y == 28) {
																				cout << char(223);
																			}
																			else
																				if ((X == 119 || X == 120 || X == 121) && (Y == 27 || Y == 28) || (X == 118 || X == 122) && Y == 27) {
																					cout << char(219);
																				}
																				else
																					if ((X == 119 || X == 120 || X == 121) && Y == 26) {
																						cout << char(220);
																					}
																					else
																						if ((X == 118 || X == 122) && Y == 28) {
																							cout << char(223);
																						}
																						else
																							if ((X == 119 || X == 120 || X == 121) && (Y == 16 || Y == 15) || (X == 118 || X == 122) && Y == 15) {
																								cout << char(219);
																							}
																							else
																								if ((X == 119 || X == 120 || X == 121) && Y == 14) {
																									cout << char(220);
																								}
																								else
																									if ((X == 118 || X == 122) && Y == 16) {
																										cout << char(223);
																									}
																									else
																										if ((X == 119 || X == 120 || X == 121) && (Y == 22 || Y == 21) || (X == 118 || X == 122) && Y == 21) {
																											cout << char(219);
																										}
																										else
																											if ((X == 119 || X == 120 || X == 121) && Y == 20) {
																												cout << char(220);
																											}
																											else
																												if ((X == 118 || X == 122) && Y == 22) {
																													cout << char(223);
																												}
																												else
																													if (X == 98 && (Y == 39 || Y == 40)) {
																														cout << char(219);
																													}
																													else
																														if ((X == 98 || X == 99 || X == 100 || X == 101 || X == 102) && Y == 41) {
																															cout << char(223);
																														}
																														else
																															if (X == 104 && (Y == 39 || Y == 40)) {
																																cout << char(219);
																															}
																															else
																																if ((X == 104 || X == 105 || X == 106 || X == 107 || X == 108) && (Y == 41 || Y == 39)) {
																																	cout << char(223);
																																}
																																else
																																	if ((X == 105 || X == 106 || X == 107) && Y == 40) {
																																		cout << char(223);
																																	}
																																	else
																																		if ((X == 110 || X == 114) && Y == 39) {
																																			cout << char(220);
																																		}
																																		else
																																			if ((X == 110 || X == 114) && Y == 40) {
																																				cout << char(219);
																																			}
																																			else
																																				if ((X == 111 || X == 112 || X == 113) && (Y == 39 || Y == 41)) {
																																					cout << char(223);
																																				}
																																				else
																																					cout << " ";
							}
							else
								cout << " ";
	}
	else
		if (contL == 1) {
			if (R == 1) {
				color(hConsole, ColorD[0]);
				if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && (Y == 33) || (X == 81 || X == 132) && (Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32) || (X == 82 || X == 131) && (Y == 11 || Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32)) {
					cout << char(219);
				}
				else
					if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && (Y == 11) || (X == 105 || X == 109) && Y == 22 || (X == 82 || X == 131) && (Y == 11 || Y == 33)) {
						cout << char(223);
					}
					else
						if ((X == 106 || X == 107 || X == 108) && (Y == 21 || Y == 22) || (X == 105 || X == 109) && Y == 21) {
							cout << char(219);
						}
						else
							if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && Y == 10 || (X == 81 || X == 132) && Y == 11 || (X == 106 || X == 107 || X == 108) && Y == 20) {
								cout << char(220);
							}
							else
								if (X == 95 && Y == 40) {
									cout << char(220);
								}
								else
									if ((X == 96 || X == 97 || X == 98) && Y == 41) {
										cout << char(223);
									}
									else
										if (X == 99 && (Y == 39 || Y == 40)) {
											cout << char(219);
										}
										else
											if ((X == 101 || X == 105) && (Y == 39 || Y == 40)) {
												cout << char(219);
											}
											else
												if ((X == 102 || X == 103 || X == 104) && Y == 41) {
													cout << char(223);
												}
												else
													if ((X == 107 || X == 111) && Y == 39) {
														cout << char(220);
													}
													else
														if ((X == 107 || X == 111) && Y == 40) {
															cout << char(219);
														}
														else
															if ((X == 107 || X == 111) && Y == 41) {
																cout << char(223);
															}
															else
																if ((X == 108 || X == 109 || X == 110) && (Y == 39 || Y == 40)) {
																	cout << char(223);
																}
																else
																	if ((X == 113 || X == 117) && (Y == 39 || Y == 40)) {
																		cout << char(219);
																	}
																	else
																		if ((X == 113 || X == 117) && Y == 41) {
																			cout << char(223);
																		}
																		else
																			if (X == 114 && Y == 39) {
																				cout << char(220);
																			}
																			else
																				if (X == 115 && Y == 40) {
																					cout << char(223);
																				}
																				else
																					if (X == 116 && Y == 40) {
																						cout << char(220);
																					}
																					else
																						cout << " ";

			}
			else
				if (R == 2) {
					color(hConsole, ColorD[1]);
					if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && (Y == 33) || (X == 81 || X == 132) && (Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32) || (X == 82 || X == 131) && (Y == 11 || Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32)) {
						cout << char(219);
					}
					else
						if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && (Y == 11) || (X == 82 || X == 131) && (Y == 11 || Y == 33)) {
							cout << char(223);
						}
						else
							if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && Y == 10 || (X == 81 || X == 132) && Y == 11) {
								cout << char(220);
							}
							else
								if ((X == 92 || X == 93 || X == 94) && (Y == 16 || Y == 15) || (X == 91 || X == 95) && Y == 15) {
									cout << char(219);
								}
								else
									if ((X == 92 || X == 93 || X == 94) && Y == 14) {
										cout << char(220);
									}
									else
										if ((X == 91 || X == 95) && Y == 16) {
											cout << char(223);
										}
										else
											if ((X == 119 || X == 120 || X == 121) && (Y == 27 || Y == 28) || (X == 118 || X == 122) && Y == 27) {
												cout << char(219);
											}
											else
												if ((X == 119 || X == 120 || X == 121) && Y == 26) {
													cout << char(220);
												}
												else
													if ((X == 118 || X == 122) && Y == 28) {
														cout << char(223);
													}
													else
														if (X == 95 && Y == 40) {
															cout << char(220);
														}
														else
															if ((X == 96 || X == 97 || X == 98) && Y == 41) {
																cout << char(223);
															}
															else
																if (X == 99 && (Y == 39 || Y == 40)) {
																	cout << char(219);
																}
																else
																	if ((X == 101 || X == 105) && (Y == 39 || Y == 40)) {
																		cout << char(219);
																	}
																	else
																		if ((X == 102 || X == 103 || X == 104) && Y == 41) {
																			cout << char(223);
																		}
																		else
																			if ((X == 107 || X == 111) && Y == 39) {
																				cout << char(220);
																			}
																			else
																				if ((X == 107 || X == 111) && Y == 40) {
																					cout << char(219);
																				}
																				else
																					if ((X == 107 || X == 111) && Y == 41) {
																						cout << char(223);
																					}
																					else
																						if ((X == 108 || X == 109 || X == 110) && (Y == 39 || Y == 40)) {
																							cout << char(223);
																						}
																						else
																							if ((X == 113 || X == 117) && (Y == 39 || Y == 40)) {
																								cout << char(219);
																							}
																							else
																								if ((X == 113 || X == 117) && Y == 41) {
																									cout << char(223);
																								}
																								else
																									if (X == 114 && Y == 39) {
																										cout << char(220);
																									}
																									else
																										if (X == 115 && Y == 40) {
																											cout << char(223);
																										}
																										else
																											if (X == 116 && Y == 40) {
																												cout << char(220);
																											}
																											else
																												cout << " ";
				}
				else
					if (R == 3) {
						color(hConsole, ColorD[2]);
						if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && (Y == 33) || (X == 81 || X == 132) && (Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32) || (X == 82 || X == 131) && (Y == 11 || Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32)) {
							cout << char(219);
						}
						else
							if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && (Y == 11) || (X == 105 || X == 109) && Y == 22 || (X == 82 || X == 131) && (Y == 11 || Y == 33)) {
								cout << char(223);
							}
							else
								if ((X == 106 || X == 107 || X == 108) && (Y == 21 || Y == 22) || (X == 105 || X == 109) && Y == 21) {
									cout << char(219);
								}
								else
									if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && Y == 10 || (X == 81 || X == 132) && Y == 11 || (X == 106 || X == 107 || X == 108) && Y == 20) {
										cout << char(220);
									}
									else
										if ((X == 92 || X == 93 || X == 94) && (Y == 16 || Y == 15) || (X == 91 || X == 95) && Y == 15) {
											cout << char(219);
										}
										else
											if ((X == 92 || X == 93 || X == 94) && Y == 14) {
												cout << char(220);
											}
											else
												if ((X == 91 || X == 95) && Y == 16) {
													cout << char(223);
												}
												else
													if ((X == 119 || X == 120 || X == 121) && (Y == 27 || Y == 28) || (X == 118 || X == 122) && Y == 27) {
														cout << char(219);
													}
													else
														if ((X == 119 || X == 120 || X == 121) && Y == 26) {
															cout << char(220);
														}
														else
															if ((X == 118 || X == 122) && Y == 28) {
																cout << char(223);
															}
															else
																if (X == 95 && Y == 40) {
																	cout << char(220);
																}
																else
																	if ((X == 96 || X == 97 || X == 98) && Y == 41) {
																		cout << char(223);
																	}
																	else
																		if (X == 99 && (Y == 39 || Y == 40)) {
																			cout << char(219);
																		}
																		else
																			if ((X == 101 || X == 105) && (Y == 39 || Y == 40)) {
																				cout << char(219);
																			}
																			else
																				if ((X == 102 || X == 103 || X == 104) && Y == 41) {
																					cout << char(223);
																				}
																				else
																					if ((X == 107 || X == 111) && Y == 39) {
																						cout << char(220);
																					}
																					else
																						if ((X == 107 || X == 111) && Y == 40) {
																							cout << char(219);
																						}
																						else
																							if ((X == 107 || X == 111) && Y == 41) {
																								cout << char(223);
																							}
																							else
																								if ((X == 108 || X == 109 || X == 110) && (Y == 39 || Y == 40)) {
																									cout << char(223);
																								}
																								else
																									if ((X == 113 || X == 117) && (Y == 39 || Y == 40)) {
																										cout << char(219);
																									}
																									else
																										if ((X == 113 || X == 117) && Y == 41) {
																											cout << char(223);
																										}
																										else
																											if (X == 114 && Y == 39) {
																												cout << char(220);
																											}
																											else
																												if (X == 115 && Y == 40) {
																													cout << char(223);
																												}
																												else
																													if (X == 116 && Y == 40) {
																														cout << char(220);
																													}
																													else
																														cout << " ";
					}
					else
						if (R == 4) {
							color(hConsole, ColorD[3]);
							if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && (Y == 33) || (X == 81 || X == 132) && (Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32) || (X == 82 || X == 131) && (Y == 11 || Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32)) {
								cout << char(219);
							}
							else
								if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && (Y == 11) || (X == 82 || X == 131) && (Y == 11 || Y == 33)) {
									cout << char(223);
								}
								else
									if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && Y == 10 || (X == 81 || X == 132) && Y == 11) {
										cout << char(220);
									}
									else
										if ((X == 92 || X == 93 || X == 94) && (Y == 16 || Y == 15) || (X == 91 || X == 95) && Y == 15) {
											cout << char(219);
										}
										else
											if ((X == 92 || X == 93 || X == 94) && Y == 14) {
												cout << char(220);
											}
											else
												if ((X == 91 || X == 95) && Y == 16) {
													cout << char(223);
												}
												else
													if ((X == 92 || X == 93 || X == 94) && (Y == 27 || Y == 28) || (X == 91 || X == 95) && Y == 27) {
														cout << char(219);
													}
													else
														if ((X == 92 || X == 93 || X == 94) && Y == 26) {
															cout << char(220);
														}
														else
															if ((X == 91 || X == 95) && Y == 28) {
																cout << char(223);
															}
															else
																if ((X == 119 || X == 120 || X == 121) && (Y == 27 || Y == 28) || (X == 118 || X == 122) && Y == 27) {
																	cout << char(219);
																}
																else
																	if ((X == 119 || X == 120 || X == 121) && Y == 26) {
																		cout << char(220);
																	}
																	else
																		if ((X == 118 || X == 122) && Y == 28) {
																			cout << char(223);
																		}
																		else
																			if ((X == 119 || X == 120 || X == 121) && (Y == 16 || Y == 15) || (X == 118 || X == 122) && Y == 15) {
																				cout << char(219);
																			}
																			else
																				if ((X == 119 || X == 120 || X == 121) && Y == 14) {
																					cout << char(220);
																				}
																				else
																					if ((X == 118 || X == 122) && Y == 16) {
																						cout << char(223);
																					}
																					else
																						if (X == 95 && Y == 40) {
																							cout << char(220);
																						}
																						else
																							if ((X == 96 || X == 97 || X == 98) && Y == 41) {
																								cout << char(223);
																							}
																							else
																								if (X == 99 && (Y == 39 || Y == 40)) {
																									cout << char(219);
																								}
																								else
																									if ((X == 101 || X == 105) && (Y == 39 || Y == 40)) {
																										cout << char(219);
																									}
																									else
																										if ((X == 102 || X == 103 || X == 104) && Y == 41) {
																											cout << char(223);
																										}
																										else
																											if ((X == 107 || X == 111) && Y == 39) {
																												cout << char(220);
																											}
																											else
																												if ((X == 107 || X == 111) && Y == 40) {
																													cout << char(219);
																												}
																												else
																													if ((X == 107 || X == 111) && Y == 41) {
																														cout << char(223);
																													}
																													else
																														if ((X == 108 || X == 109 || X == 110) && (Y == 39 || Y == 40)) {
																															cout << char(223);
																														}
																														else
																															if ((X == 113 || X == 117) && (Y == 39 || Y == 40)) {
																																cout << char(219);
																															}
																															else
																																if ((X == 113 || X == 117) && Y == 41) {
																																	cout << char(223);
																																}
																																else
																																	if (X == 114 && Y == 39) {
																																		cout << char(220);
																																	}
																																	else
																																		if (X == 115 && Y == 40) {
																																			cout << char(223);
																																		}
																																		else
																																			if (X == 116 && Y == 40) {
																																				cout << char(220);
																																			}
																																			else
																																				cout << " ";
						}
						else
							if (R == 5) {
								color(hConsole, ColorD[4]);
								if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && (Y == 33) || (X == 81 || X == 132) && (Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32) || (X == 82 || X == 131) && (Y == 11 || Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32)) {
									cout << char(219);
								}
								else
									if ((X == 105 || X == 109) && Y == 22 || (X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && (Y == 11) || (X == 82 || X == 131) && (Y == 11 || Y == 33)) {
										cout << char(223);
									}
									else
										if ((X == 106 || X == 107 || X == 108) && Y == 20 || (X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && Y == 10 || (X == 81 || X == 132) && Y == 11) {
											cout << char(220);
										}
										else
											if ((X == 106 || X == 107 || X == 108) && (Y == 21 || Y == 22) || (X == 105 || X == 109) && Y == 21 || (X == 92 || X == 93 || X == 94) && (Y == 16 || Y == 15) || (X == 91 || X == 95) && Y == 15) {
												cout << char(219);
											}
											else
												if ((X == 92 || X == 93 || X == 94) && Y == 14) {
													cout << char(220);
												}
												else
													if ((X == 91 || X == 95) && Y == 16) {
														cout << char(223);
													}
													else
														if ((X == 92 || X == 93 || X == 94) && (Y == 27 || Y == 28) || (X == 91 || X == 95) && Y == 27) {
															cout << char(219);
														}
														else
															if ((X == 92 || X == 93 || X == 94) && Y == 26) {
																cout << char(220);
															}
															else
																if ((X == 91 || X == 95) && Y == 28) {
																	cout << char(223);
																}
																else
																	if ((X == 119 || X == 120 || X == 121) && (Y == 27 || Y == 28) || (X == 118 || X == 122) && Y == 27) {
																		cout << char(219);
																	}
																	else
																		if ((X == 119 || X == 120 || X == 121) && Y == 26) {
																			cout << char(220);
																		}
																		else
																			if ((X == 118 || X == 122) && Y == 28) {
																				cout << char(223);
																			}
																			else
																				if ((X == 119 || X == 120 || X == 121) && (Y == 16 || Y == 15) || (X == 118 || X == 122) && Y == 15) {
																					cout << char(219);
																				}
																				else
																					if ((X == 119 || X == 120 || X == 121) && Y == 14) {
																						cout << char(220);
																					}
																					else
																						if ((X == 118 || X == 122) && Y == 16) {
																							cout << char(223);
																						}
																						else
																							if (X == 95 && Y == 40) {
																								cout << char(220);
																							}
																							else
																								if ((X == 96 || X == 97 || X == 98) && Y == 41) {
																									cout << char(223);
																								}
																								else
																									if (X == 99 && (Y == 39 || Y == 40)) {
																										cout << char(219);
																									}
																									else
																										if ((X == 101 || X == 105) && (Y == 39 || Y == 40)) {
																											cout << char(219);
																										}
																										else
																											if ((X == 102 || X == 103 || X == 104) && Y == 41) {
																												cout << char(223);
																											}
																											else
																												if ((X == 107 || X == 111) && Y == 39) {
																													cout << char(220);
																												}
																												else
																													if ((X == 107 || X == 111) && Y == 40) {
																														cout << char(219);
																													}
																													else
																														if ((X == 107 || X == 111) && Y == 41) {
																															cout << char(223);
																														}
																														else
																															if ((X == 108 || X == 109 || X == 110) && (Y == 39 || Y == 40)) {
																																cout << char(223);
																															}
																															else
																																if ((X == 113 || X == 117) && (Y == 39 || Y == 40)) {
																																	cout << char(219);
																																}
																																else
																																	if ((X == 113 || X == 117) && Y == 41) {
																																		cout << char(223);
																																	}
																																	else
																																		if (X == 114 && Y == 39) {
																																			cout << char(220);
																																		}
																																		else
																																			if (X == 115 && Y == 40) {
																																				cout << char(223);
																																			}
																																			else
																																				if (X == 116 && Y == 40) {
																																					cout << char(220);
																																				}
																																				else
																																					cout << " ";
							}
							else
								if (R == 6) {
									color(hConsole, ColorD[5]);
									if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && (Y == 33) || (X == 81 || X == 132) && (Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32) || (X == 82 || X == 131) && (Y == 11 || Y == 12 || Y == 13 || Y == 14 || Y == 15 || Y == 16 || Y == 17 || Y == 18 || Y == 19 || Y == 20 || Y == 21 || Y == 22 || Y == 23 || Y == 24 || Y == 25 || Y == 26 || Y == 27 || Y == 28 || Y == 29 || Y == 30 || Y == 31 || Y == 32)) {
										cout << char(219);
									}
									else
										if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && (Y == 11) || (X == 82 || X == 131) && (Y == 11 || Y == 33)) {
											cout << char(223);
										}
										else
											if ((X == 83 || X == 84 || X == 85 || X == 86 || X == 87 || X == 88 || X == 89 || X == 90 || X == 91 || X == 92 || X == 93 || X == 94 || X == 95 || X == 96 || X == 97 || X == 98 || X == 99 || X == 100 || X == 101 || X == 102 || X == 103 || X == 104 || X == 105 || X == 106 || X == 107 || X == 108 || X == 109 || X == 110 || X == 111 || X == 112 || X == 113 || X == 114 || X == 115 || X == 116 || X == 117 || X == 118 || X == 119 || X == 120 || X == 121 || X == 122 || X == 123 || X == 124 || X == 125 || X == 126 || X == 127 || X == 128 || X == 129 || X == 130) && Y == 10 || (X == 81 || X == 132) && Y == 11) {
												cout << char(220);
											}
											else
												if ((X == 92 || X == 93 || X == 94) && (Y == 16 || Y == 15) || (X == 91 || X == 95) && Y == 15) {
													cout << char(219);
												}
												else
													if ((X == 92 || X == 93 || X == 94) && Y == 14) {
														cout << char(220);
													}
													else
														if ((X == 91 || X == 95) && Y == 16) {
															cout << char(223);
														}
														else
															if ((X == 92 || X == 93 || X == 94) && (Y == 22 || Y == 21) || (X == 91 || X == 95) && Y == 21) {
																cout << char(219);
															}
															else
																if ((X == 92 || X == 93 || X == 94) && Y == 20) {
																	cout << char(220);
																}
																else
																	if ((X == 91 || X == 95) && Y == 22) {
																		cout << char(223);
																	}
																	else
																		if ((X == 92 || X == 93 || X == 94) && (Y == 27 || Y == 28) || (X == 91 || X == 95) && Y == 27) {
																			cout << char(219);
																		}
																		else
																			if ((X == 92 || X == 93 || X == 94) && Y == 26) {
																				cout << char(220);
																			}
																			else
																				if ((X == 91 || X == 95) && Y == 28) {
																					cout << char(223);
																				}
																				else
																					if ((X == 119 || X == 120 || X == 121) && (Y == 27 || Y == 28) || (X == 118 || X == 122) && Y == 27) {
																						cout << char(219);
																					}
																					else
																						if ((X == 119 || X == 120 || X == 121) && Y == 26) {
																							cout << char(220);
																						}
																						else
																							if ((X == 118 || X == 122) && Y == 28) {
																								cout << char(223);
																							}
																							else
																								if ((X == 119 || X == 120 || X == 121) && (Y == 16 || Y == 15) || (X == 118 || X == 122) && Y == 15) {
																									cout << char(219);
																								}
																								else
																									if ((X == 119 || X == 120 || X == 121) && Y == 14) {
																										cout << char(220);
																									}
																									else
																										if ((X == 118 || X == 122) && Y == 16) {
																											cout << char(223);
																										}
																										else
																											if ((X == 119 || X == 120 || X == 121) && (Y == 22 || Y == 21) || (X == 118 || X == 122) && Y == 21) {
																												cout << char(219);
																											}
																											else
																												if ((X == 119 || X == 120 || X == 121) && Y == 20) {
																													cout << char(220);
																												}
																												else
																													if ((X == 118 || X == 122) && Y == 22) {
																														cout << char(223);
																													}
																													else
																														if (X == 95 && Y == 40) {
																															cout << char(220);
																														}
																														else
																															if ((X == 96 || X == 97 || X == 98) && Y == 41) {
																																cout << char(223);
																															}
																															else
																																if (X == 99 && (Y == 39 || Y == 40)) {
																																	cout << char(219);
																																}
																																else
																																	if ((X == 101 || X == 105) && (Y == 39 || Y == 40)) {
																																		cout << char(219);
																																	}
																																	else
																																		if ((X == 102 || X == 103 || X == 104) && Y == 41) {
																																			cout << char(223);
																																		}
																																		else
																																			if ((X == 107 || X == 111) && Y == 39) {
																																				cout << char(220);
																																			}
																																			else
																																				if ((X == 107 || X == 111) && Y == 40) {
																																					cout << char(219);
																																				}
																																				else
																																					if ((X == 107 || X == 111) && Y == 41) {
																																						cout << char(223);
																																					}
																																					else
																																						if ((X == 108 || X == 109 || X == 110) && (Y == 39 || Y == 40)) {
																																							cout << char(223);
																																						}
																																						else
																																							if ((X == 113 || X == 117) && (Y == 39 || Y == 40)) {
																																								cout << char(219);
																																							}
																																							else
																																								if ((X == 113 || X == 117) && Y == 41) {
																																									cout << char(223);
																																								}
																																								else
																																									if (X == 114 && Y == 39) {
																																										cout << char(220);
																																									}
																																									else
																																										if (X == 115 && Y == 40) {
																																											cout << char(223);
																																										}
																																										else
																																											if (X == 116 && Y == 40) {
																																												cout << char(220);
																																											}
																																											else
																																												cout << " ";
								}
								else
									cout << " ";
		}
		else
			if (contL == 2) {
				contL = 0;
			}


}