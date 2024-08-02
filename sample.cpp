#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string>
#include <vector>
#include <iostream>

#define _USE_MATH_DEFINES
#include <math.h>
#include <glm/gtc/matrix_transform.hpp>

#ifndef F_PI
#define F_PI		((float)(M_PI))
#define F_2_PI		((float)(2.f*F_PI))
#define F_PI_2		((float)(F_PI/2.f))
#endif


#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif

#include "glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "glut.h"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// CS 450 / 550 --Fall Quarter 2023
// 100 Points
// Due : December Somethingth
// Final Project

// Author:			Joseph McFadden

#pragma region FunctionSet

// function prototypes:

void	Animate();
void	Display();
void	DoAxesMenu(int);
void	DoColorMenu(int);
void	DoDepthBufferMenu(int);
void	DoDepthFightingMenu(int);
void	DoDepthMenu(int);
void	DoDebugMenu(int);
void	DoMainMenu(int);
void	DoProjectMenu(int);
void	DoRasterString(float, float, float, char*);
void	DoStrokeString(float, float, float, float, char*);
float	ElapsedSeconds();
void	InitGraphics();
void	InitChessBoard();
void	InitLists();
void	InitMenus();
void	Keyboard(unsigned char, int, int);
void	MouseButton(int, int, int, int);
void	MouseMotion(int, int);
void	Reset();
void	Resize(int, int);
void	Visibility(int);
void	RenderRay();

char*   ToChar(const char*);
void	Axes(float);
void	HsvRgb(float[3], float[3]);
void	Cross(float[3], float[3], float[3]);
float	Dot(float[3], float[3]);
float	Unit(float[3], float[3]);
float	Unit(float[3]);
float*  Array3(float, float, float);
float*  MulArray3(float, float, float, float);
float*  MulArray3(float, float[]);
bool	AreVec3Equal(const glm::vec3&, const glm::vec3&, float = 0.001f);
float	GetFallingTime(float);

struct	BoundingBox;
struct	Piece;
#pragma endregion

#pragma region Colors

// window background color (rgba):

const GLfloat BLACK[] = { 0., 0., 0., 1. };
const GLfloat GRAY[] = { 0.5f, 0.5f, 0.5f, 1.0f };
const float	WHITE[] = { 1.,1.,1.,1. };


// the color numbers:
// this order must match the radio button order, which must match the order of the color names,
// 	which must match the order of the color RGB values



char* ColorNames[] =
{
	(char*)"Red",
	(char*)"Yellow",
	(char*)"Green",
	(char*)"Cyan",
	(char*)"Blue",
	(char*)"Magenta",
	(char*)"White",
};

// the color definitions:
// this order must match the menu order

const GLfloat Colors[][3] =
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
	{ 1.,1.,1. }		// white
};

#pragma endregion

#pragma region IncludedFiles

// these are here for when you need them -- just uncomment the ones you need:
#include "setmaterial.cpp"
#include "setlight.cpp"
#include "osusphere.cpp"
#include "bmptotexture.cpp"
#include "loadobjfile.cpp"

#include "keytime.cpp"
#include <stdexcept>

//#include "osucone.cpp"
//#include "osutorus.cpp"
//#include "glslprogram.cpp"
//#include "CarouselHorse0.10.550.cpp"

#pragma endregion

#pragma region Constant Global Variables

// title of these windows:
const char* WINDOWTITLE = "Joseph McFadden";
const char* GLUITITLE = "Chess with me mate?";

// what the glui package defines as true and false:
const int GLUITRUE = true;
const int GLUIFALSE = false;

// the escape key:
const int ESCAPE = 0x1b;

// initial window size:
const int viewportHeight = 700;
const int viewportWidth = 700;

// multiplication factors for input interaction:
//  (these are known from previous experience)
const float ANGFACT = 1.f;
const float SCLFACT = 0.005f;

// allowable scale factors:
const float zoomSpeed = 5.0f;
const float minFov = 0.05f;		// Minimun FOV
const float maxFov = 150.0f;	// Maximum FOV

// scroll wheel button values:
const int SCROLL_WHEEL_UP = 3;
const int SCROLL_WHEEL_DOWN = 4;

// active mouse buttons (or them together):
const int LEFT = 4;
const int MIDDLE = 2;
const int RIGHT = 1;

// line width for the axes:
const GLfloat AXES_WIDTH = 3.;

const float piecePosScale = 20.;
const float observableScale = 1.;
const float fallingDuration = 20.0f;

int		DebugOn;				// != 0 means to print debugging info
float	Time;
float	nowTime;
const int MSEC = 10000;
float elapsedALLTIME;
#pragma endregion 

#pragma region Enums

// Chess piece types
enum PieceType
{
	PAWN,
	KNIGHT,
	BISHOP,
	ROOK,
	QUEEN,
	KING
};

std::string PieceTypeToString(PieceType type) {
	switch (type) {
	case PAWN:   return "Pawn";
	case KNIGHT: return "Knight";
	case BISHOP: return "Bishop";
	case ROOK:   return "Rook";
	case QUEEN:  return "Queen";
	case KING:   return "King";
	default:     return "Unknown";
	}
}


// which projection:
enum Projections
{
	ORTHO,
	PERSP
};

// which button:
enum ButtonVals
{
	RESET,
	QUIT
};

// not implemented
enum Views
{
	INSIDE_OUT,
	ANGLED,
	HEAD_ON
};

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA
};
#pragma endregion

#pragma region Global Classes
struct BoundingBox
{
	glm::vec3 origin;        // Origin of the bounding box
	glm::vec3 extents;       // Distance from the origin to the min/max points

	// Constructor to initialize with minimum and maximum values
	BoundingBox(const glm::vec3& minimum, const glm::vec3& maximum, glm::vec3 o)
		: extents((maximum - minimum) * 0.5f), origin(o)
	{
	}

	BoundingBox(const glm::vec3& ex, glm::vec3 o) : origin(o), extents(ex)
	{}

	BoundingBox() 
	{
		origin = glm::vec3(0);
		extents = glm::vec3(0);
	}

	// Calculate and return the minimum point
	glm::vec3 min() const {
		return origin - extents;
	}

	// Calculate and return the maximum point
	glm::vec3 max() const {
		return origin + extents;
	}

	// Calculate and return the minimum point
	glm::vec3 observedMin() const {
		glm::vec3 minVec = min();
		return glm::vec3(minVec.x * observableScale, minVec.y, minVec.z * observableScale);
	}

	glm::vec3 observedMax() const {
		glm::vec3 maxVec = max();
		return glm::vec3(maxVec.x * observableScale, maxVec.y, maxVec.z * observableScale);
	}

	void move()
	{
		try {
			glTranslatef(origin.x, origin.y, origin.z);
			
		}
		catch (const std::exception& e) {
			throw std::runtime_error("Error with keeping origin pointer");
		}
	}

	bool Intersects(const BoundingBox& other) {
		// Check if there is no overlap in any dimension
		if (max().x < other.min().x || min().x > other.max().x) return false;
		if (max().y < other.min().y || min().y > other.max().y) return false;
		if (max().z < other.min().z || min().z > other.max().z) return false;

		return true; // Overlaps in all dimensions
	}
};

struct Piece
{
	std::string name;
	std::string TileLocation;
	char* file;
	float scale;
	GLuint displayList;
	GLuint bboxList;
	unsigned int texObject;
	bool isSelected;
	bool isHovered; // to be imlemented
	glm::vec3 origin; // essentially x/z 
	glm::vec3 dimensions; // Dimensions for the bounding box
	BoundingBox boundingBox;


	// Additional game-related properties
	PieceType type; 
	bool isWhite;
	glm::vec3 color; // For rendering the piece with a specific color

	glm::vec3 startPosition; // Starting position for the animation
	glm::vec3 endPosition; // Ending position for the animation
	float startTime; // Time when the animation starts
	bool isAnimating = false; // Flag to indicate if the piece is currently animating

	float fallingStartTime;
	
	bool isDying = false;
	BoundingBox pieceIamKillingsBox;
	
	bool isFading = false;
	float opacity = 1.0f;
	bool isDead = false;

	/** 
	 * @brief Construct a new Chess Piece object
	 *
	 * @param nm Name of the chess piece (e.g., "White Pawn").
	 * @param fl Filename for the object (e.g., "pieces/pawn.obj").
	 * @param scl Scale for rendering the piece.
	 * @param isW Flag indicating if the piece is white.
	 * @param loc origin of the piece in world coordinates.
	 * @param dims Dimensions of the piece's bounding box.
	 * @param tp Type of the piece using the PieceType enum.
	 */
	Piece(const std::string& nm, const char*& charfl, float scl, bool isW,
		const glm::vec3& loc, const glm::vec3& dims, PieceType tp, std::string(tileLoc))
		: name(nm), file(ToChar(charfl)), scale(scl), isSelected(false), isHovered(false),
		origin(loc), dimensions(dims), type(tp), isWhite(isW), TileLocation(tileLoc)
	{
		initself();
	}

	Piece(const std::string& nm, const std::string& fl, float scl, bool isW,
		const glm::vec3& loc, const glm::vec3& dims, PieceType tp , std::string(tileLoc))
		: name(nm), file(ToChar(fl.c_str())), scale(scl), isSelected(false), isHovered(false),
		origin(loc), dimensions(dims), type(tp), isWhite(isW), TileLocation(tileLoc)
	{
		initself();
	}

	void initself()
	{
		// Initialize displayList, texObject, and other properties here
		displayList = 0;
		texObject = 0;

		// The color can be set based on whether the piece is white or black
		color = isWhite ? glm::vec3(1.0f, 1.0f, 1.0f) : glm::vec3(0.0f, 0.0f, 0.0f);


	}

	bool IsAboutToKill() {
		return boundingBox.Intersects(pieceIamKillingsBox);
	}

	void startAnimation(const glm::vec3& endPos) {
		startPosition = origin; // Current position becomes the start position
		endPosition = endPos; // Set the end position
		startTime = Time; // Record the start time
		isAnimating = true; // Set the animation flag
	}

	void endAnimation() {
		// clear 
		startPosition = glm::vec3(); 
		endPosition = glm::vec3();
		startTime = -11111; 
		isAnimating = false; // Set the animation flag
	}

	void startFadeAnimation() {
		isFading = true;
		opacity = 1.0f; // Start fully opaque
	}

	void startDeathAnimation()
	{
		isDying = true;
		fallingStartTime = elapsedALLTIME;
	}

	void endDeathAnimation() 
	{
		isDying = false;
		startFadeAnimation();
	}

	void moveSelf(glm::vec3 translation)
	{
		glTranslatef(translation[0], translation[1], translation[2]);
		boundingBox.origin = translation;
		// update all the properties for the object
	}

	void callSelf()
	{
		// Save the current matrix state:
		glPushMatrix();

		// Disable lighting temporarily: for textyres I think
		//glDisable(GL_LIGHTING);

		if (isDying)
		{
			float elapsedTime = (elapsedALLTIME - fallingStartTime) ; // Scale to milliseconds
			float currentAngle = GetFallingTime(elapsedTime); // Scale back to seconds

            glRotatef(currentAngle, 1.f, 0., 0.); // Handle the rotation using keyframes

            if (elapsedTime >= fallingDuration) {
                endDeathAnimation();
            }
		}
		else if (isFading) {
			opacity -= 0.01f;
			if (opacity <= 0) {
				opacity = 0;
				isFading = false;
				isDead = true;
			}
		}

		moveSelf(origin);

		if (isWhite)
			glColor4f(WHITE[0], WHITE[1], WHITE[2], opacity);
		else
			glColor4f(BLACK[0], BLACK[1], BLACK[2], opacity);

		// Draw the sphere:
		glCallList(displayList);

		// Restore lighting state: for textyres I think
		//glEnable(GL_LIGHTING);

		// Restore the previous matrix state:
		glPopMatrix();
	}

	void callBox()
	{
		// Save the current matrix state:
		glPushMatrix();

		// Disable lighting temporarily:
		glDisable(GL_LIGHTING);

		boundingBox.move();

		glColor3f(1.f, 0.f, 0.0f);

		// Draw the sphere:
		glCallList(bboxList);

		// Restore lighting state:
		glEnable(GL_LIGHTING);

		// Restore the previous matrix state:
		glPopMatrix();
	}

	GLuint DrawPiece(glm::vec3& min, glm::vec3& max, glm::vec3& origin)
	{
		GLuint dl = glGenLists(1);
		glNewList(dl, GL_COMPILE);
		SetMaterial(color[0], color[1], color[2], 70.f);
		//glBindTexture(GL_TEXTURE_2D, texObject);
		glPushMatrix();
		//glScalef(scale, scale, scale);

		LoadObjFile(file, min, max, origin);

		glPopMatrix();
		glEndList();

		return dl;
	}

	void makeMoves() {
		if (!isAnimating) return; // Do nothing if not animating

		float duration = .1f; // Total animation duration in seconds
		float elapsed = Time - startTime;
		if (elapsed >= duration) {
			elapsed = duration; // Clamp elapsed time
			isAnimating = false; // End animation
			origin = endPosition; // Ensure the piece reaches the final position
		}

		float normalizedTime = elapsed / duration;
		origin = glm::mix(startPosition, endPosition, normalizedTime);

		// Sine wave adjustments for smooth movement
		float waveAmplitude = 0.5f; // Amplitude of the sine wave
		float waveFrequency = 2.0f; // Frequency of the sine wave
		float sineValue = waveAmplitude * sin(waveFrequency * glm::pi<float>() * normalizedTime);

		// Assuming the y-coordinate is the vertical axis. Change this if your coordinate system is different.
		origin.y += sineValue;

	/*	if (pieceIamKillingsBox != nullptr) {
			if (!pieceIamKillingsBox->isDying && IsAboutToKill()) {
				pieceIamKillingsBox->startDeathAnimation();
			}
		}*/

		if (AreVec3Equal(endPosition,origin))
			endAnimation();
	}

	// Note this must be called before any translation is made if you want the original bounding box
	GLuint DrawBoundingBox()
	{
		glm::vec3 min = boundingBox.observedMin();
		glm::vec3 max = boundingBox.observedMax();

		GLuint dl = glGenLists(1);
		glNewList(dl, GL_COMPILE);
		glColor3f(1.0f, .0f, .0f); // Set color to red for the bounding box lines
		glDisable(GL_LIGHTING); // Disable lighting for drawing lines
		glDisable(GL_TEXTURE_2D); // Disable textures
		glPushMatrix();

		// Draw the bounding box as line loops
		glBegin(GL_LINE_LOOP);
		glVertex3f(min.x, min.y, min.z);
		glVertex3f(max.x, min.y, min.z);
		glVertex3f(max.x, max.y, min.z);
		glVertex3f(min.x, max.y, min.z);
		glEnd();

		glBegin(GL_LINE_LOOP);
		glVertex3f(min.x, min.y, max.z);
		glVertex3f(max.x, min.y, max.z);
		glVertex3f(max.x, max.y, max.z);
		glVertex3f(min.x, max.y, max.z);
		glEnd();

		glBegin(GL_LINES);
		glVertex3f(min.x, min.y, min.z);
		glVertex3f(min.x, min.y, max.z);
		glVertex3f(max.x, min.y, min.z);
		glVertex3f(max.x, min.y, max.z);
		glVertex3f(max.x, max.y, min.z);
		glVertex3f(max.x, max.y, max.z);
		glVertex3f(min.x, max.y, min.z);
		glVertex3f(min.x, max.y, max.z);
		glEnd();

		glPopMatrix();
		glEnable(GL_LIGHTING); // Re-enable lighting
		glEnable(GL_TEXTURE_2D); // Re-enable textures
		glEndList();

		return dl;
	}
};

struct Tile {
	glm::vec3 position;       // Position of the tile center
	BoundingBox boundingBox;  // Bounding box for interaction
	std::string notation;     // Chess notation for the tile (e.g., A1, A2, ..., H8)
	glm::vec3 extents;        // Extents of the tile

	Tile(const glm::vec3& pos, const glm::vec3& size, const std::string& note)
		: position(pos), extents(size * 0.5f), notation(note)
	{
		boundingBox = BoundingBox(extents, pos);
	}
};

struct Board {
	std::vector<Tile> tiles;  // Container for all chess tiles
	GLuint boardList;

	// Function to initialize the board
	void initBoard(float piecePosScale) {
		tiles.clear();
		glm::vec3 tileSize(1.0f * piecePosScale, 0.2f * piecePosScale, 1.0f * piecePosScale); // Size of each tile

		// Assuming 8x8 board
		for (int row = 0; row < 8; ++row) {
			for (int col = 0; col < 8; ++col) {
				glm::vec3 tilePosition((col - 3.5f) * piecePosScale, 0.0f, (row - 3.5f) * piecePosScale);
				std::string notation = char('A' + col) + std::to_string(8 - row); // Generating notation
				tiles.emplace_back(tilePosition, tileSize, notation);
			}
		}

		if (DebugOn == 1)
			LogTiles();
	}

	// Function to create a display list for rendering the chessboard
	void drawBoard() {
		// Generate a new display list ID
		boardList = glGenLists(1);

		// Compile the display list
		glNewList(boardList, GL_COMPILE);

		// Set the colors for white and black squares
		glm::vec3 whiteColor(1.0f, 1.0f, 1.0f);
		glm::vec3 blackColor(0.1f, 0.1f, 0.1f);

		// Loop through the tiles in the chess board and render squares
		for (int row = 0; row < 8; ++row) {
			for (int col = 0; col < 8; ++col) {
				// Get the corresponding tile for the current row and column
				const Tile& tile = tiles[row * 8 + col];

				glm::vec3 squareColor = ((row + col) % 2 == 0) ? whiteColor : blackColor;

				glColor3fv(glm::value_ptr(squareColor));
				glBegin(GL_QUADS);
				glVertex3f(tile.position.x - tile.extents.x, 0.0f, tile.position.z - tile.extents.z);
				glVertex3f(tile.position.x + tile.extents.x, 0.0f, tile.position.z - tile.extents.z);
				glVertex3f(tile.position.x + tile.extents.x, 0.0f, tile.position.z + tile.extents.z);
				glVertex3f(tile.position.x - tile.extents.x, 0.0f, tile.position.z + tile.extents.z);
				glEnd();
			}
		}

		// End the display list compilation
		glEndList();
	}

	void callBoard()
	{
		// Save the current matrix state:
		glPushMatrix();

		// Disable lighting temporarily:
		glDisable(GL_LIGHTING);

		// Draw the sphere:
		glCallList(boardList);

		// Restore lighting state:
		glEnable(GL_LIGHTING);

		// Restore the previous matrix state:
		glPopMatrix();
	}

	void LogTiles() const {
		std::cout << "Tile Details:" << std::endl;
		for (const Tile& tile : tiles) {
			std::cout << "Notation: " << tile.notation << std::endl;
			std::cout << "X Location: " << tile.position.x << std::endl;
			std::cout << "Y Location: " << tile.position.y << std::endl;
			std::cout << "Z Location: " << tile.position.z << std::endl;
			std::cout << "-------------------" << std::endl;
		}
	}
};

struct Chess
{
	Board board;
	std::vector<Piece> WhitePieces;
	std::vector<Piece> BlackPieces;
	Piece* firstPieceSel;
	Piece* secondPieceSel;
	Tile* tileSelected;

	Chess(const std::vector<Piece>& wp, const std::vector<Piece>& bp)
		: WhitePieces(wp), BlackPieces(bp)
	{}

	Chess() {}

	void InitChess() {
		// Initialize the board first
		board.initBoard(piecePosScale);

		// Initialize Pieces
		InitPieces();

		// Update piece locations based on tile notation
		for (auto& piece : WhitePieces) {
			UpdateLocationBasedOnTile(piece);
		}
		for (auto& piece : BlackPieces) {
			UpdateLocationBasedOnTile(piece);
		}
	}

	void UpdateLocationBasedOnTile(Piece& cp) {
		for (const auto& tile : board.tiles) {
			if (tile.notation == cp.TileLocation) {
				cp.origin = tile.position;
				break;
			}
		}
	}

	void InitPieces() {
		// Tile notations for the first row of White and Black pieces
		const std::string whiteRowNotations[8] = { "A1", "B1", "C1", "D1", "E1", "F1", "G1", "H1" };
		const std::string blackRowNotations[8] = { "A8", "B8", "C8", "D8", "E8", "F8", "G8", "H8" };
		const PieceType pieceTypes[8] = { ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK };

		// Initialize White and Black Pieces
		for (int i = 0; i < 8; ++i) {
			std::string pieceCount;
			if (pieceTypes[i] != KING && pieceTypes[i] != QUEEN)
			{
				pieceCount = " " + std::to_string(i < 4 ? 1 : 2);
			}

			std::string whitePieceName = "White " + PieceTypeToString(pieceTypes[i]) + pieceCount;
			std::string blackPieceName = "Black " + PieceTypeToString(pieceTypes[i]) + pieceCount;;

			WhitePieces.push_back(Piece(whitePieceName, "pieces/" + PieceTypeToString(pieceTypes[i]) + ".obj", 1.f, true, glm::vec3(), glm::vec3(1.f, 1.f, 1.f), pieceTypes[i], whiteRowNotations[i]));
			BlackPieces.push_back(Piece(blackPieceName, "pieces/" + PieceTypeToString(pieceTypes[i]) + ".obj", 1.f, false, glm::vec3(), glm::vec3(1.f, 1.f, 1.f), pieceTypes[i], blackRowNotations[i]));
		}

		// Initialize Pawns for White and Black
		for (char col = 'A'; col <= 'H'; col++) {
			std::string whitePawnName = "White Pawn " + std::to_string(col - 'A' + 1);
			std::string blackPawnName = "Black Pawn " + std::to_string(col - 'A' + 1);
			char* pawnFile = "pieces/Pawn.obj";
			std::string whiteTileLocation = std::string(1, col) + "2"; // Row 2 for white pawns
			std::string blackTileLocation = std::string(1, col) + "7"; // Row 7 for black pawns

			WhitePieces.push_back(Piece(whitePawnName, pawnFile, 1.f, true, glm::vec3(), glm::vec3(1.f, 1.f, 1.f), PAWN, whiteTileLocation));
			BlackPieces.push_back(Piece(blackPawnName, pawnFile, 1.f, false, glm::vec3(), glm::vec3(1.f, 1.f, 1.f), PAWN, blackTileLocation));
		}
	}

	Piece* CheckPieceIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDir)
	{
		Piece* closestPiece = nullptr;
		float closestDistance = std::numeric_limits<float>::max(); // Initialize with the maximum possible float value

		// Check intersection with WhitePieces
		for (size_t i = 0; i < WhitePieces.size(); ++i) {
			float distance = 0.0f;
			if (RayIntersectsBox(rayOrigin, rayDir, WhitePieces[i].boundingBox, distance)) {
				if (distance < closestDistance) {
					closestDistance = distance;
					closestPiece = &WhitePieces[i];
				}
			}
		}

		// Check intersection with BlackPieces
		for (size_t i = 0; i < BlackPieces.size(); ++i) {
			float distance = 0.0f;
			if (RayIntersectsBox(rayOrigin, rayDir, BlackPieces[i].boundingBox, distance)) {
				if (distance < closestDistance) {
					closestDistance = distance;
					closestPiece = &BlackPieces[i];
				}
			}
		}


		return closestPiece; // Will be nullptr if no intersection is found
	}

	Tile* CheckTileIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDir)
	{
		// Check intersection with WhitePieces
		for (size_t i = 0; i < board.tiles.size(); ++i) {
			float distance = 0.0f;
			if (RayIntersectsBox(rayOrigin, rayDir, board.tiles[i].boundingBox, distance)) {
				return &board.tiles[i];
			}
		}

		return nullptr; // Will be null if no intersection is found
	}

	bool RayIntersectsBox(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const BoundingBox& box, float& distance)
	{
		glm::vec3 invDir = 1.0f / rayDir;
		glm::vec3 t0s = (box.observedMin() - rayOrigin) * invDir;
		glm::vec3 t1s = (box.observedMax() - rayOrigin) * invDir;

		glm::vec3 tmin = glm::min(t0s, t1s);
		glm::vec3 tmax = glm::max(t0s, t1s);

		float tminMax = glm::max(glm::max(tmin.x, tmin.y), tmin.z);
		float tmaxMin = glm::min(glm::min(tmax.x, tmax.y), tmax.z);

		if (tmaxMin < 0 || tminMax > tmaxMin)
			return false; // No intersection

		distance = tminMax;
		return true;
	}

	Piece* doIMovePiece() {
		if (firstPieceSel != nullptr && secondPieceSel != nullptr) // this is an attack!
		{
			if (AreVec3Equal(firstPieceSel->origin, secondPieceSel->origin))
			{
				firstPieceSel = nullptr;
				secondPieceSel = nullptr;
				return nullptr;
			}
			return firstPieceSel;
		}
		else if (firstPieceSel != nullptr && tileSelected != nullptr) // this is a move
		{
			if (AreVec3Equal(firstPieceSel->origin, tileSelected->position))
			{
				firstPieceSel = nullptr;
				tileSelected = nullptr;
				return nullptr;
			}
			return firstPieceSel;
		}
		return nullptr; 
	}

	int HandleMove()
	{
		if (firstPieceSel != nullptr && secondPieceSel != nullptr) // this is an attack!
		{
			glm::vec3 destination = secondPieceSel->origin; // Assuming this gives the end position
			firstPieceSel->startAnimation(destination);
			firstPieceSel->TileLocation = secondPieceSel->TileLocation;
		}
		else if (firstPieceSel != nullptr && tileSelected != nullptr)
		{
			glm::vec3 destination = tileSelected->position; // Assuming this gives the end position
			firstPieceSel->startAnimation(destination);
			firstPieceSel->TileLocation = tileSelected->notation;
		}
		
		else if (firstPieceSel == nullptr && tileSelected == nullptr) {
			fprintf(stderr, "Error: Both pointers are empty.\n");
			return 0;
		}
		else if (firstPieceSel == nullptr) {
			fprintf(stderr, "Error: firstPieceSel pointer is empty.\n");
			return 0;
		}
		else {
			fprintf(stderr, "Error: tileSelected pointer is empty.\n");
			return 0;
		}
		
		return 1;
	}
	
	int HandleAttack() {
		return 0;
	}

	Piece* getPieceByName(const std::string& pieceName) {
		// Search in white pieces
		for (Piece& piece : WhitePieces) {
			if (piece.name == pieceName) {
				return &piece; // Return a pointer to the found piece
			}
		}

		// Search in black pieces
		for (Piece& piece : BlackPieces) {
			if (piece.name == pieceName) {
				return &piece; // Return a pointer to the found piece
			}
		}

		return nullptr; // No piece found with the given name
	}


#pragma region Moving Rules / Logic

	bool IsPathClear(const std::string& start, const std::string& end) {
		// Check if it's a diagonal move
		bool isDiagonal = (abs(start[0] - end[0]) == abs(start[1] - end[1]));

		// Check if it's a straight move (same column or same row)
		bool isStraight = (start[0] == end[0]) || (start[1] == end[1]);

		if (!isDiagonal && !isStraight) {
			return false; // Neither diagonal nor straight move
		}

		// Handle diagonal moves
		if (isDiagonal) {
			// Determine the direction of the diagonal
			int xDirection = (start[0] < end[0]) ? 1 : -1;
			int yDirection = (start[1] < end[1]) ? 1 : -1;

			// Check each tile along the diagonal path
			char currentX = start[0] + xDirection;
			char currentY = start[1] + yDirection;
			while (currentX != end[0] && currentY != end[1]) {
				std::string location = std::string(1, currentX) + std::string(1, currentY);
				Piece* piece = GetPieceAtLocation(location);

				if (piece != nullptr) {
					// There's a piece in the way, so the path is not clear
					return false;
				}

				// Move to the next tile along the diagonal path
				currentX += xDirection;
				currentY += yDirection;
			}
		}

		// Handle straight moves
		if (isStraight) {
			// Check if it's a horizontal move (same column)
			if (start[0] == end[0]) {
				int startY = start[1] - '0';
				int endY = end[1] - '0';
				int yDirection = (startY < endY) ? 1 : -1;

				// Check each tile along the horizontal path
				char currentY = start[1] + yDirection;
				while (currentY != end[1]) {
					std::string location = std::string(1, start[0]) + std::string(1, currentY);
					Piece* piece = GetPieceAtLocation(location);

					if (piece != nullptr) {
						// There's a piece in the way, so the path is not clear
						return false;
					}

					// Move to the next tile along the horizontal path
					currentY += yDirection;
				}
			}
			else {
				// Handle vertical moves
				int startX = start[0] - 'A' + 1;
				int endX = end[0] - 'A' + 1;
				int xDirection = (startX < endX) ? 1 : -1;

				// Check each tile along the vertical path
				char currentX = start[0];
				while (currentX != end[0]) {
					std::string location = std::string(1, currentX) + end.substr(1);
					Piece* piece = GetPieceAtLocation(location);

					if (piece != nullptr) {
						// There's a piece in the way, so the path is not clear
						return false;
					}

					// Move to the next tile along the vertical path
					currentX += xDirection;
				}
			}
		}

		// If the loop completes without finding any pieces in the path, the path is clear
		return true;
	}

	Piece* GetPieceAtLocation(const std::string& location) {
		// Iterate through all pieces and find the one with the matching TileLocation
		for (Piece& piece : WhitePieces) {
			if (piece.TileLocation == location) {
				return &piece;
			}
		}

		for (Piece& piece : BlackPieces) {
			if (piece.TileLocation == location) {
				return &piece;
			}
		}

		return nullptr; // No piece found at the given location
	}

	bool ValidatePawnMove(const Piece& piece, const std::string& targetLocation, const Piece* targetPiece) {
		// Determine the direction of movement based on whether it's a white or black pawn
		int direction = (piece.isWhite) ? 1 : -1;

		// Calculate the difference in rows (numbers) between the start and target locations
		int rowDifference = (targetLocation[1] - piece.TileLocation[1]) * direction;
		int colDifference = targetLocation[0] - piece.TileLocation[0];

		// Check for a valid vertical move
		if (colDifference == 0) {
			if (rowDifference == 1 && targetPiece == nullptr) {
				// Regular single move forward
				return true;
			}
			else if (rowDifference == 2 && ((piece.isWhite && piece.TileLocation[1] == '2') || (!piece.isWhite && piece.TileLocation[1] == '7')) && targetPiece == nullptr) {
				// First move allows for a double move forward from the starting position
				return true;
			}
		}

		// Check for a valid diagonal capture
		if (std::abs(colDifference) == 1 && rowDifference == 1 && targetPiece != nullptr) {
			// Diagonal capture
			return true;
		}

		// Invalid move
		return false;
	}

	bool ValidateKnightMove(const Piece& piece, const std::string& targetLocation) {
		// Calculate the absolute horizontal and vertical distance between current and target locations
		int deltaX = abs(targetLocation[0] - piece.TileLocation[0]);
		int deltaY = abs(targetLocation[1] - piece.TileLocation[1]);

		// Check if the move is in an L-shape (2 squares in one direction and 1 square in another)
		if ((deltaX == 2 && deltaY == 1) || (deltaX == 1 && deltaY == 2)) {
			// Get the piece at the target location
			Piece* targetPiece = GetPieceAtLocation(targetLocation);

			if (targetPiece == nullptr) {
				// No piece at the target location, allow the move
				return true;
			}
			else if (targetPiece->isWhite != piece.isWhite) {
				// Piece at the target location is of a different color, allow capturing
				return true;
			}
		}

		// Piece at the target location is of the same color or the move is invalid
		return false;
	}


	bool ValidateBishopMove(const Piece& piece, const std::string& targetLocation) {
		

		if (!IsPathClear(piece.TileLocation, targetLocation))
			return false;
		// Get the piece at the target location
		Piece* targetPiece = GetPieceAtLocation(targetLocation);


		if (targetPiece == nullptr) {
			// No piece at the target location, allow the move
			return true;
		}
		else if (targetPiece->isWhite != piece.isWhite) {
			// Piece at the target location is of a different color, allow capturing
			return true;
		}

		// Piece at the target location is of the same color, disallow the move
		return false;
	}


	bool ValidateRookMove(const Piece& piece, const std::string& targetLocation) {
		if (!IsPathClear(piece.TileLocation, targetLocation))
			return false;

		// Get the piece at the target location
		Piece* targetPiece = GetPieceAtLocation(targetLocation);

		if (targetPiece == nullptr) {
			// No piece at the target location, allow the move
			return true;
		}
		else if (targetPiece->isWhite != piece.isWhite) {
			// Piece at the target location is of a different color, allow capturing
			return true;
		}

		// Piece at the target location is of the same color, disallow the move
		return false;
	}


	bool ValidateQueenMove(const Piece& piece, const std::string& targetLocation) {

		if (!IsPathClear(piece.TileLocation, targetLocation))
			return false;

		// Get the piece at the target location
		Piece* targetPiece = GetPieceAtLocation(targetLocation);

		if (targetPiece == nullptr) {
			// No piece at the target location, allow the move
			return true;
		}
		else if (targetPiece->isWhite != piece.isWhite) {
			// Piece at the target location is of a different color, allow capturing
			return true;
		}

		// Piece at the target location is of the same color, disallow the move
		return false;
	}


	bool ValidateKingMove(const Piece& piece, const std::string& targetLocation) {
		// Get the piece at the target location
		Piece* targetPiece = GetPieceAtLocation(targetLocation);

		if (targetPiece == nullptr) {
			// No piece at the target location, allow the move
			return true;
		}
		else if (targetPiece->isWhite != piece.isWhite) {
			// Piece at the target location is of a different color, allow capturing
			return true;
		}

		// Piece at the target location is of the same color, disallow the move
		return false;
	}


	bool IsMoveLegal(const Piece& piece, const std::string& targetLocation) {
		Piece* targetPiece = GetPieceAtLocation(targetLocation);

		switch (piece.type) {
		case PieceType::PAWN:
			return ValidatePawnMove(piece, targetLocation, targetPiece);
		case PieceType::KNIGHT:
			return ValidateKnightMove(piece, targetLocation);
		case PieceType::BISHOP:
			return ValidateBishopMove(piece, targetLocation);
		case PieceType::ROOK:
			return ValidateRookMove(piece, targetLocation);
		case PieceType::QUEEN:
			return ValidateQueenMove(piece, targetLocation);
		case PieceType::KING:
			return ValidateKingMove(piece, targetLocation);
		default:
			return false;
		}
	}

#pragma endregion

	void HandleSelection(const glm::vec3& rayOrigin, const glm::vec3& rayDir)
	{
		if (DebugOn == 1) {
			fprintf(stderr, "Ray origin: %f, %f, %f Ray direction: %f, %f, %f\n",
				rayOrigin.x, rayOrigin.y, rayOrigin.z,
				rayDir.x, rayDir.y, rayDir.z);
		}

		Piece* cp = CheckPieceIntersection(rayOrigin, rayDir);
		Tile* tile = CheckTileIntersection(rayOrigin, rayDir);
		if (tile != nullptr)
			fprintf(stderr, "Clicked on %s\n", tile->notation.c_str());
		// check if first piece slected only select a piece if no 1st selected
		if (firstPieceSel == nullptr)
		{

			if (cp != nullptr) { 
				firstPieceSel = cp;
				fprintf(stderr, "Clicked on %s\n", firstPieceSel->name.c_str());
				firstPieceSel->isSelected = true;
			}
			else {
				tileSelected = nullptr;
				fprintf(stderr, "No piece was clicked.\n");
			}
		}
		else {
			if (cp != nullptr) { // piece is selcted and might attack
				if (cp->name == firstPieceSel->name)
				{
					firstPieceSel == nullptr;
					firstPieceSel->isSelected = false;
				}
				else {
					secondPieceSel = cp;
					fprintf(stderr, "Clicked on %s\n", secondPieceSel->name.c_str());
					secondPieceSel->isSelected = true;

					fprintf(stderr, "Attempting to attack %s to %s...\n", firstPieceSel->name.c_str(), secondPieceSel->name.c_str());

					if (IsMoveLegal(*firstPieceSel, secondPieceSel->TileLocation.c_str())) {
						// Handle the legal move
						firstPieceSel->pieceIamKillingsBox = secondPieceSel->boundingBox;
						HandleMove();
						HandleAttack(); 

						fprintf(stderr, "Attack Move is legal!\n");
					}
					else {
						secondPieceSel = nullptr;
						firstPieceSel = nullptr;
						fprintf(stderr, "Move is not legal!\n");
					}
				}
			}
			else 
			{
				tileSelected = tile;
				if (tileSelected != nullptr)
				{
					fprintf(stderr, "Clicked on %s\n", tileSelected->notation.c_str());

					// Attempting to make a move
					fprintf(stderr, "Attempting to move %s to %s...\n", firstPieceSel->name.c_str(), tileSelected->notation.c_str());

					// Check move legality
					if (IsMoveLegal(*firstPieceSel, tileSelected->notation.c_str())) {
						// Handle the legal move
						HandleMove();
						fprintf(stderr, "Move is legal!\n");
					}
					else {
						// Move is not legal
						tileSelected = nullptr;
						firstPieceSel = nullptr;
						fprintf(stderr, "Move is not legal!\n");
					}
				}
				else {
					tileSelected = nullptr;
					firstPieceSel = nullptr;
					secondPieceSel = nullptr;
					fprintf(stderr, "No piece or tile was clicked.\n");
				}
			}
		}
	}
};

#pragma endregion

#pragma region grid
#define XSIDE	10000			// length of the x side of the grid
#define X0      (-XSIDE/2.)		// where one side starts
#define NX		1000			// how many points in x
#define DX		( XSIDE/(float)NX )	// change in x between the points

#define YGRID	-4.f
// XY base for grid
//#define YSIDE	1000			// length of the z side of the grid
//#define Y0      (-YSIDE/2.)		// where one side starts
//#define NY		1000			// how many points in z
//#define DY		( YSIDE/(float)NY )	// change in z between the points

#define ZSIDE	10000			// length of the z side of the grid
#define Z0      (-ZSIDE/2.)		// where one side starts
#define NZ	1000			// how many points in z
#define DZ	( ZSIDE/(float)NZ )	// change in z between the points
#pragma endregion

#pragma region  Lighting

// fog parameters:
const GLfloat FOGCOLOR[4] = { .0f, .0f, .0f, 1.f };
const GLenum  FOGMODE     = GL_LINEAR;
const GLfloat FOGDENSITY  = 0.30f;
const GLfloat FOGSTART    = 1.5f;
const GLfloat FOGEND      = 4.f;

// for lighting:

const float LIGHT_RADIUS = 20.0f;
const float LIGHT_HEIGHT = 90.0f;

#pragma endregion

#pragma region animation

// for animation:
		// 10000 milliseconds = 10 seconds
bool	Frozen;
int lastPrintTime = 0;

#pragma endregion

#pragma region ComplileOptions

// what options should we compile-in?
// in general, you don't need to worry about these
// i compile these in to show class examples of things going wrong
//#define DEMO_Z_FIGHTING
//#define DEMO_DEPTH_BUFFER

#pragma endregion

#pragma region Non-Constant Global Variables

// non-constant global variables:

int		ActiveButton;			// current button that is down

GLuint PawnList; 
//PlanetType PlanetNow;
bool TextureType;
bool LightMode;
//GLuint	VenusTex, EarthTex, MarsTex, JupiterTex, SaturnTex, UranusTex, NeptuneTex;
GLuint	AxesList, GridList;				
int		AxesOn;					// != 0 means to draw the axes
int		DepthCueOn;				// != 0 means to use intensity depth cueing
int		DepthBufferOn;			// != 0 means to use the z-buffer
int		DepthFightingOn;		// != 0 means to force the creation of z-fighting
int		MainWindow;				// window id for main graphics window
int		NowColor;				// index into Colors[ ]
int		NowProjection = PERSP;	// ORTHO or PERSP
float	Scale;					// scaling factor
int		ShadowsOn;				// != 0 means to turn shadows on
//float	Time;					// used for animation, this has a value between 0. and 1.

int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees
float	LastPrintTime = -1.0f;
int		CurrentView = ANGLED;	// Default view
Keytimes fallingAnimation;

float LightAngle; // angle of light in radians
float xLightPos;  // x position of the light
float zLightPos;  // z position of the light
int LightColorNow = 6;
bool isSpotLight = false;  // Default is Point Light


GLfloat light_direction[];
glm::mat4 viewMatrix;
glm::mat4 projMatrix;
float aspectRatio;

glm::vec3 cameraPos = glm::vec3(0, 500, 200); // Camera position
glm::vec3 cameraTarget = glm::vec3(0, 0, 0); // Look-at point
glm::vec3 upVector = glm::vec3(0, 1, 0);     // Up vector
float fov = 1.f;                          // Field of view

Chess chess;

glm::vec3& rayOrigin = glm::vec3(69);
glm::vec3& rayDir = glm::vec3(69);
#pragma endregion

#pragma region CustomMathClasses
// utility to create an array from 3 separate values:

float *
Array3( float a, float b, float c )
{
	static float array[4];

	array[0] = a;
	array[1] = b;
	array[2] = c;
	array[3] = 1.;
	return array;
}

// utility to create an array from a multiplier and an array:

float *
MulArray3( float factor, float array0[ ] )
{
	static float array[4];

	array[0] = factor * array0[0];
	array[1] = factor * array0[1];
	array[2] = factor * array0[2];
	array[3] = 1.;
	return array;
}


float *
MulArray3(float factor, float a, float b, float c )
{
	static float array[4];

	float* abc = Array3(a, b, c);
	array[0] = factor * abc[0];
	array[1] = factor * abc[1];
	array[2] = factor * abc[2];
	array[3] = 1.;
	return array;
}
#pragma endregion

#pragma region weird post includes 

Keytimes LightAngleKT;
Keytimes Ypos;
Keytimes ViewAngle;
Keytimes Grow;
Keytimes ShakeX;
Keytimes RedChannel, GreenChannel, BlueChannel;
Keytimes XPositionKT, ZPositionKT;
Keytimes TiltKT;

#pragma endregion

#pragma region Main
// main program:

int
main( int argc, char *argv[ ] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since glutInit might
	// pull some command line arguments out)

	glutInit( &argc, argv );

	// setup all the graphics stuff:

	InitGraphics( );

	// create the display lists that **will not change**:

	Reset( );

	chess.InitChess();
	InitLists( );
	// init all the global variables used by Display( ):
	// this will also post a redisplay

	

	// setup all the user interface stuff:

	InitMenus( );

	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow( MainWindow );
	glutMainLoop( );

	// glutMainLoop( ) never actually returns
	// the following line is here to make the compiler happy:

	return 0;
}

#pragma endregion

// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutPostRedisplay( ) do it
void
Animate( )
{
	// put animation stuff in here -- change some global variables for Display( ) to find:

	int ms = glutGet(GLUT_ELAPSED_TIME);
	elapsedALLTIME = ms/(float)(100);
	ms %= (MSEC);
	Time = (float)ms / (float)(MSEC);

	Piece* cp = chess.doIMovePiece();
	if (cp != nullptr)
	{
		if (chess.secondPieceSel != nullptr) {
			if (!chess.secondPieceSel->isDying && cp->IsAboutToKill())
			{
				chess.secondPieceSel->startDeathAnimation();
			}
		}
		cp->makeMoves();
	}

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

#pragma region callMethods
void callLight()
{
	// Get the interpolated light angle value for the current time
	LightAngle = LightAngleKT.GetValue(nowTime);

	// Convert the angle to a position
	xLightPos = LIGHT_RADIUS * cos(LightAngle);
	zLightPos = LIGHT_RADIUS * sin(LightAngle);

	// Apply light position for rendering
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);


	if (isSpotLight) {
		SetSpotLight(GL_LIGHT0, xLightPos, LIGHT_HEIGHT, zLightPos, 0.0f, -1.0f, .0f, Colors[LightColorNow][0], Colors[LightColorNow][1], Colors[LightColorNow][2]);
	}
	else {
		SetPointLight(GL_LIGHT0, xLightPos, LIGHT_HEIGHT, zLightPos, Colors[LightColorNow][0], Colors[LightColorNow][1], Colors[LightColorNow][2]);
	}
}

void callPieces()
{
	for (int type = PAWN; type < KNIGHT; type++)
	{
		// Initialize white pieces of this type
		for (Piece& whitePiece : chess.WhitePieces) {
			if (!whitePiece.isDead)
			{
				whitePiece.callSelf();
				if (DebugOn != 0)
				{
					whitePiece.callBox();
				}
			}
		}
		// Initialize black pieces of this type
		for (Piece& blackPiece : chess.BlackPieces) {
			if (!blackPiece.isDead) {
				blackPiece.callSelf();
				if (DebugOn != 0)
				{
					blackPiece.callBox();
				}
			}
		}
	}
}

void callPawn(float x, float y, float z) {
	glPushMatrix(); // Save the current matrix state


	glTranslatef(x, y, z);

	glScalef(1.f, 1.f, 1.f);

	glColor3f(0.f, 0.f, 0.0f);
	glCallList(PawnList);

	glPopMatrix(); // Restore the previous matrix state
}
#pragma endregion

void
AnimateLight() {
	LightAngle = LightAngleKT.GetValue(nowTime); // Get interpolated angle

	xLightPos = LIGHT_RADIUS * cos(LightAngle);
	zLightPos = LIGHT_RADIUS * sin(LightAngle);
}

#pragma region CameraSet

void UpdateCamera() {
	// Set view matrix using the camera position, target, and up vector:
	viewMatrix = glm::lookAt(cameraPos, cameraTarget, upVector);

	aspectRatio = (float)glutGet(GLUT_WINDOW_WIDTH) / (float)glutGet(GLUT_WINDOW_HEIGHT);

	float adjustedFOV = glm::radians(fov) / Scale;
	adjustedFOV = glm::clamp(adjustedFOV, glm::radians(1.0f), glm::radians(179.0f)); // Clamp to valid range
	projMatrix = glm::perspective(adjustedFOV, aspectRatio, 0.1f, 1000.f);
}

void defineCamera() {
	// set the viewport to be a square centered in the window :

	GLsizei vx = glutGet(GLUT_WINDOW_WIDTH);
	GLsizei vy = glutGet(GLUT_WINDOW_HEIGHT);
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = (vx - v) / 2;
	GLint yb = (vy - v) / 2;


	glViewport(xl, yb, v, v);


	// set the viewing volume:
	// remember that the Z clipping  values are given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	aspectRatio = (float)vx / (float)vy;
	if (NowProjection == ORTHO)
	{
		glOrtho(-2.f, 2.f, -2.f, 2.f, 0.1f, 1000.f);
		projMatrix = glm::ortho(-2.f, 2.f, -2.f, 2.f, 0.1f, 1000.f);
	}
	else
	{
		float adjustedFOV = glm::radians(fov) / Scale;
		adjustedFOV = glm::clamp(adjustedFOV, glm::radians(1.0f), glm::radians(179.0f)); // Clamp to valid range

		gluPerspective(fov, aspectRatio, 0.1f, 1000.f);
		projMatrix = glm::perspective(adjustedFOV, aspectRatio, 0.1f, 1000.f);
	}

	// Set view matrix:
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z, cameraTarget.x, cameraTarget.y, cameraTarget.z, upVector.x, upVector.y, upVector.z);
	viewMatrix = glm::lookAt(cameraPos, cameraTarget, upVector);

	// Rotate the scene:
	glRotatef((GLfloat)Yrot, 0.f, 1.f, 0.f);
	glRotatef((GLfloat)Xrot, 1.f, 0.f, 0.f);

	// Uniformly scale the scene:
	if (Scale < minFov)
	{
		Scale = minFov;
	}
	glScalef((GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale);
}

#pragma endregion

// draw the complete scene:
void
Display( )
{
	/*if (DebugOn != 0)
		fprintf(stderr, "Starting Display.\n");*/
	int ms = glutGet(GLUT_ELAPSED_TIME);
	ms %= MSEC;
	nowTime = (float)ms / 10000.0f;

	// set which window we want to do the graphics into:
	glutSetWindow( MainWindow );

	// erase the background:
	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable( GL_DEPTH_TEST );
#ifdef DEMO_DEPTH_BUFFER
	if( DepthBufferOn == 0 )
		glDisable( GL_DEPTH_TEST );
#endif


	// specify shading to be flat:

	glShadeModel( GL_FLAT );

	defineCamera();

	if (DebugOn != 0 && rayDir != glm::vec3(69))
	{
		RenderRay();
	}
	// set the fog parameters:

	if( DepthCueOn != 0 )
	{
		glFogi( GL_FOG_MODE, FOGMODE );
		glFogfv( GL_FOG_COLOR, FOGCOLOR );
		glFogf( GL_FOG_DENSITY, FOGDENSITY );
		glFogf( GL_FOG_START, FOGSTART );
		glFogf( GL_FOG_END, FOGEND );
		glEnable( GL_FOG );
	}
	else
	{
		glDisable( GL_FOG );
	}

	callLight();


	if( AxesOn != 0 )
	{
		glColor3fv( &Colors[NowColor][0] );
		glCallList( AxesList );
	}


	glEnable( GL_NORMALIZE );

	//glPushAttrib(GL_CURRENT_BIT); // Save the current color
	//glColor3f(0.5f, 0.5f, 0.5f);
	//glCallList(GridList);
	//glPopAttrib(); // Restore the previous color


	if (TextureType)
		glEnable(GL_TEXTURE_2D);
	else
		glDisable(GL_TEXTURE_2D);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	
	if (LightMode)
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	else
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	
	callPieces();

	chess.board.callBoard();
	
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);


#ifdef DEMO_Z_FIGHTING
	if( DepthFightingOn != 0 )
	{
		glPushMatrix( );
			glRotatef( 90.f,   0.f, 1.f, 0.f );
			glCallList( BoxList );
		glPopMatrix( );
	}
#endif

	glDisable( GL_DEPTH_TEST );
	glColor3f( 0.f, 1.f, 1.f );
	//DoRasterString( 0.f, 1.f, 0.f, (char *)"Text That Moves" );


	// draw some gratuitous text that is fixed on the screen:
	//
	// the projection matrix is reset to define a scene whose
	// world coordinate system goes from 0-100 in each axis
	//
	// this is called "percent units", and is just a convenience
	//
	// the modelview matrix is reset to identity as we don't
	// want to transform these coordinates
	glDisable( GL_DEPTH_TEST );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluOrtho2D( 0.f, 100.f,     0.f, 100.f );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	glColor3f( 1.f, 1.f, 1.f );

	// swap the double-buffered framebuffers:
	glutSwapBuffers( );

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !
	glFlush( );
}

#pragma region MenuMethods
void SetCameraView(int view) {
	CurrentView = view;
	glutPostRedisplay();
}

void
DoAxesMenu( int id )
{
	AxesOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoColorMenu( int id )
{
	NowColor = id - RED;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDebugMenu( int id )
{
	DebugOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthBufferMenu( int id )
{
	DepthBufferOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthFightingMenu( int id )
{
	DepthFightingOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthMenu( int id )
{
	DepthCueOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// main menu callback:
void
DoMainMenu( int id )
{
	switch( id )
	{
		case RESET:
			Reset( );
			break;

		case QUIT:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:
			glutSetWindow( MainWindow );
			glFinish( );
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;

		default:
			fprintf( stderr, "Don't know what to do with Main Menu ID %d\n", id );
	}

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoProjectMenu( int id )
{
	if (CurrentView != INSIDE_OUT)
	{
		NowProjection = id;

		glutSetWindow(MainWindow);
		glutPostRedisplay();
	}
}

#pragma endregion

#pragma region ToBeOrganizedMiscMethods
// use glut to display a string of characters using a raster font:
void
DoRasterString( float x, float y, float z, char *s )
{
	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );

	char c;			// one character to print
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}


// use glut to display a string of characters using a stroke font:
void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
	glPushMatrix( );
		glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
		float sf = ht / ( 119.05f + 33.33f );
		glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
		char c;			// one character to print
		for( ; ( c = *s ) != '\0'; s++ )
		{
			glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
		}
	glPopMatrix( );
}


// return the number of seconds since the start of the program:
float
ElapsedSeconds( )
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:

	return (float)ms / 1000.f;
}

#pragma endregion

#pragma region Draw
// Function to draw a red circle in the X-Y plane
void drawCircle() {
	float theta;
	glColor3f(1.0f, 0.0f, 0.0f);  // Set color to red
	glBegin(GL_LINE_LOOP);  // Begin drawing line loop (circle)
	for (int i = 0; i < 360; i++) {
		theta = i * F_PI / 180;  // Convert degrees to radians
		glVertex3f(2.0 * cos(theta), 2.0 * sin(theta), 0.0);
	}
	glEnd();  // End drawing
}

void drawGrid() {
	GridList = glGenLists(1);
	glNewList(GridList, GL_COMPILE);
	SetMaterial(0.6f, 0.6f, 0.6f, 30.f);
	glNormal3f(0., 1., 0.);
	for (int i = 0; i < NZ; i++)
	{
		glBegin(GL_QUAD_STRIP);
		for (int j = 0; j < NX; j++)
		{
			glVertex3f(X0 + DX * (float)j, YGRID, Z0 + DZ * (float)(i + 0));
			glVertex3f(X0 + DX * (float)j, YGRID, Z0 + DZ * (float)(i + 1));
		}
		glEnd();
	}
	/*for (int i = 0; i < NY; i++)
	{
		glBegin(GL_QUAD_STRIP);
		for (int j = 0; j < NX; j++)
		{
			glVertex3f(X0 + DX * (float)j, Y0 + DY * (float)(i + 0), ZGRID);
			glVertex3f(X0 + DX * (float)j, Y0 + DY * (float)(i + 1), ZGRID);
		}
		glEnd();
	}*/
	glEndList();
}

void drawPieces()
{
	// for each piece in the list of piece types call the piece type and then for the rest of the objects 
	// TODO when other piece objs are downladed change to KING
	for (int type = PAWN; type <= KING; type++ )
	{
		// Initialize white pieces of this type
		GLuint dl = 0;
		GLuint bdl = 0;

		glm::vec3 min = glm::vec3(1.e+37f);
		glm::vec3 max = glm::vec3(-1.e+37f);
		glm::vec3 origin = glm::vec3(0.);

		for (Piece& whitePiece : chess.WhitePieces) {

			if (whitePiece.type == type) {
				if (dl == 0)
				{
					dl = whitePiece.DrawPiece(min, max, origin);
				}

				whitePiece.boundingBox = BoundingBox(min, max, origin);
				if (DebugOn != 0 && bdl == 0)
				{
					bdl = whitePiece.DrawBoundingBox();
				}

				
				whitePiece.displayList = dl;
				whitePiece.bboxList = bdl;
			}
		}
		dl = 0;
		bdl = 0;
		min = glm::vec3(1.e+37f);
		max = glm::vec3(-1.e+37f);

		// Initialize black pieces of this type
		for (Piece& blackPiece : chess.BlackPieces) {

			if (blackPiece.type == type) {
				if (dl == 0)
				{
					dl = blackPiece.DrawPiece(min, max, origin);
				}

				blackPiece.boundingBox = BoundingBox(min, max, origin);
				if (DebugOn != 0 && bdl == 0)
				{
					bdl = blackPiece.DrawBoundingBox();
				}
				
				blackPiece.displayList = dl;
				blackPiece.bboxList = bdl;
			}
		}
	}
	
}

void RenderRay()
{
	// Length of the ray for visualization
	float rayLength = 10000.0f;  // You can adjust the length as needed

	// Calculate the end point of the ray
	glm::vec3 endPoint = rayOrigin + rayDir * rayLength;

	// Save the current state of the OpenGL
	glPushAttrib(GL_ENABLE_BIT);

	// Disable lighting to make sure the color is what we set
	glDisable(GL_LIGHTING);

	// Set the color of the ray to be red for visibility
	glColor3f(1.0f, 0.0f, 0.0f);

	// Start drawing lines
	glBegin(GL_LINES);

	// Set the material properties if you're using lighting
	// SetMaterial(...);

	// Specify the two points of the line segment
	glVertex3f(rayOrigin.x, rayOrigin.y, rayOrigin.z);
	glVertex3f(endPoint.x, endPoint.y, endPoint.z);

	// Done drawing lines
	glEnd();

	// Restore the state of OpenGL back to what it was before
	glPopAttrib();
}

//void DrawPlanet(PlanetType planetType) {
//	int planetIndex = static_cast<int>(planetType);
//
//	if (planetIndex >= VENUS && planetIndex <= NEPTUNE) {
//		GLuint planet = glGenLists(1);
//		glNewList(planet, GL_COMPILE);
//		glBindTexture(GL_TEXTURE_2D, Planets[planetIndex].texObject);
//		glPushMatrix();
//		glScalef(Planets[planetIndex].scale, Planets[planetIndex].scale, Planets[planetIndex].scale);
//		glCallList(SphereDL);
//		glPopMatrix();
//		glEndList();
//
//		Planets[planetIndex].displayList = planet;
//	}
//	else {
//		fprintf(stderr, "Invalid planet type.\n");
//	}
//}

#pragma endregion

#pragma region Textures
//void LoadPlanetTexture(PlanetType planet) {
//	int width, height;
//	if (planet >= VENUS && planet <= NEPTUNE) {
//		const char* planetPath = Planets[planet].file;
//		std::string path = "planets/" + std::string(planetPath);
//		char* file = new char[path.length() + 1];
//		std::strcpy(file, path.c_str());
//
//		unsigned char* texture = BmpToTexture(file, &width, &height);
//		if (texture == NULL) {
//			fprintf(stderr, "Cannot open texture '%s'\n", file);
//		}
//		else {
//			fprintf(stderr, "Opened '%s': width = %d ; height = %d\n", file, width, height);
//
//			glGenTextures(1, &Planets[planet].texObject);
//			glBindTexture(GL_TEXTURE_2D, Planets[planet].texObject);
//			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//			glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);
//		}
//
//		delete[] file;
//	}
//	else {
//		fprintf(stderr, "Invalid planet type.\n");
//	}
//}

#pragma endregion

#pragma region keytime Anmiation 
void FallingTimes() {
	fallingAnimation.Init();

	// Scale key times based on total duration
	fallingAnimation.AddTimeValue(0.0f, 0.0f); // Start
	fallingAnimation.AddTimeValue(1.0f / 6.0f * fallingDuration, 10.0f); 
	fallingAnimation.AddTimeValue(2.0f / 6.0f * fallingDuration, 45.0f); 
	fallingAnimation.AddTimeValue(3.0f / 6.0f * fallingDuration, 89.0f); 
	fallingAnimation.AddTimeValue(4.0f / 6.0f * fallingDuration, 76.0f); 
	fallingAnimation.AddTimeValue(5.0f / 6.0f * fallingDuration, 89.0f); 
	fallingAnimation.AddTimeValue(11.0f / 12.0f * fallingDuration, 85.0f);
	fallingAnimation.AddTimeValue(6.0f / 6.0f * fallingDuration, 90.0f);
}

float GetFallingTime(float elapsedTime) {
	if (elapsedTime < 0.f || elapsedTime > fallingDuration)
		return fallingAnimation.GetLastTime();
	else
		return fallingAnimation.GetValue(elapsedTime);
}
#pragma endregion

#pragma region initMethods

// initialize the glui window:
void
InitMenus( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitMenus.\n");

	glutSetWindow( MainWindow );

	int numColors = sizeof( Colors ) / ( 3*sizeof(float) );
	int colormenu = glutCreateMenu( DoColorMenu );
	for( int i = 0; i < numColors; i++ )
	{
		glutAddMenuEntry( ColorNames[i], i );
	}

	int axesmenu = glutCreateMenu( DoAxesMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthcuemenu = glutCreateMenu( DoDepthMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthbuffermenu = glutCreateMenu( DoDepthBufferMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthfightingmenu = glutCreateMenu( DoDepthFightingMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int debugmenu = glutCreateMenu( DoDebugMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int projmenu = glutCreateMenu( DoProjectMenu );
	glutAddMenuEntry( "Orthographic",  ORTHO );
	glutAddMenuEntry( "Perspective",   PERSP );

	int viewmenu = glutCreateMenu(SetCameraView);
	glutAddMenuEntry("Inside-Out", INSIDE_OUT);
	glutAddMenuEntry("Angled", ANGLED);
	glutAddMenuEntry("Head-On", HEAD_ON);

	int mainmenu = glutCreateMenu( DoMainMenu );
	glutAddSubMenu(   "Axes",          axesmenu);
	glutAddSubMenu(   "Axis Colors",   colormenu);
	glutAddSubMenu(	"View", viewmenu);

#ifdef DEMO_DEPTH_BUFFER
	glutAddSubMenu(   "Depth Buffer",  depthbuffermenu);
#endif

#ifdef DEMO_Z_FIGHTING
	glutAddSubMenu(   "Depth Fighting",depthfightingmenu);
#endif

	glutAddSubMenu(   "Depth Cue",     depthcuemenu);
	glutAddSubMenu(   "Projection",    projmenu );
	glutAddMenuEntry( "Reset",         RESET );
	glutAddSubMenu(   "Debug",         debugmenu);
	glutAddMenuEntry( "Quit",          QUIT );

// attach the pop-up menu to the right mouse button:

	glutAttachMenu( GLUT_RIGHT_BUTTON );
}

// initialize the glut and OpenGL libraries:
//	also setup callback functions
void
InitGraphics( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitGraphics.\n");

	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	// set the initial window configuration:
	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( viewportWidth, viewportHeight );

	// open the window and set its title:
	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );

	// set the framebuffer clear values:
	glClearColor( GRAY[0], GRAY[1], GRAY[2], GRAY[3] );

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow( MainWindow );
	glutDisplayFunc( Display );
	glutReshapeFunc( Resize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc(MouseMotion);
	//glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( -1, NULL, 0 );

	FallingTimes();
	//for (int i = VENUS; i <= NEPTUNE; ++i) {
	//	LoadPlanetTexture(static_cast<PlanetType>(i));
	//}
	// setup glut to call Animate( ) every time it has
	// 	nothing it needs to respond to (which is most of the time)
	// we don't need to do this for this program, and really should set the argument to NULL
	// but, this sets us up nicely for doing animation

	glutIdleFunc( Animate );

	// init the glew package (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit( );
	if( err != GLEW_OK )
	{
		fprintf( stderr, "glewInit Error\n" );
	}
	else
		fprintf( stderr, "GLEW initialized OK\n" );
	fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

	// all other setups go here, such as GLSLProgram and KeyTime setups:

}

// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )
void
InitLists( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitLists.\n");

	glutSetWindow( MainWindow );


	drawPieces();
	chess.board.drawBoard();

	// create the axes:
	AxesList = glGenLists( 1 );
	glNewList( AxesList, GL_COMPILE );
		glLineWidth( AXES_WIDTH );
			Axes( 20 );
		glLineWidth( 1. );
	glEndList( );
}
#pragma endregion

// the keyboard callback:
void
Keyboard( unsigned char c, int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
	{
		case 'f':
		case 'F':
			Frozen = !Frozen;
			if (Frozen)
				glutIdleFunc(NULL);
			else
				glutIdleFunc(Animate);
		case 'l':
		case 'L':
			LightMode = !LightMode; // Toggle Light Mode on / off
			break;

		case 't':
		case 'T':
			TextureType = !TextureType; // Toggle texture on/off
			break;

		case 'q':
		case 'Q':
		case ESCAPE:
			DoMainMenu( QUIT );	// will not return here
			break;				// happy compiler

		case 'w':
			LightColorNow = 6; // White
			break;

		case 'r':
			LightColorNow = 0; // Red
			break;

		case 'g':
			LightColorNow = 2; // Green
			break;

		case 'b':
			LightColorNow = 4; // Blue
			break;

		case 'y':
			LightColorNow = 1; // Yellow
			break;


		default:
			fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}

	// force a call to Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

#pragma region Mouse

glm::vec3 GetRayDirection(float mouseX, float mouseY, const glm::ivec2& screenSize) {
	// Convert mouse coordinates to normalized device coordinates
	glm::vec2 mouseNDC;
	mouseNDC.x = (2.0f * mouseX) / screenSize.x - 1.0f;
	mouseNDC.y = 1.0f - (2.0f * mouseY) / screenSize.y;

	// Convert NDC to world space to get the ray direction
	glm::vec4 clipCoords = glm::vec4(mouseNDC.x, mouseNDC.y, -1.0f, 1.0f);
	glm::vec4 eyeCoords = glm::inverse(projMatrix) * clipCoords;
	eyeCoords = glm::vec4(eyeCoords.x, eyeCoords.y, -1.0f, 0.0f);
	glm::vec3 worldRayDir = glm::normalize(glm::vec3(glm::inverse(viewMatrix) * eyeCoords));

	return worldRayDir;
}

/*unproject is code I might want*/
//glm::vec3 Unproject(float x, float y,const glm::ivec2& screenSize, float planeDepth = 0.1f) {
//	// Convert screen coordinates to normalized device coordinates
//	glm::vec3 ndc;
//	ndc.x = (2.0f * x) / screenSize.x - 1.0f;
//	ndc.y = 1.0f - (2.0f * y) / screenSize.y;
//	ndc.z = planeDepth; // Assuming we're unprojecting to the far plane for simplicity
//
//	// Convert NDC to clip space
//	glm::vec4 clipSpace = glm::vec4(ndc, 1.f);
//
//	// Unproject from clip space to camera space
//	glm::mat4 invProjMatrix = glm::inverse(projMatrix);
//	glm::vec4 cameraSpace = invProjMatrix * clipSpace;
//
//	// Unproject from camera space to world space
//	glm::mat4 invViewMatrix = glm::inverse(viewMatrix);
//	glm::vec4 worldSpace = invViewMatrix * cameraSpace;
//
//	// Assuming the w component is not 0, we divide by w to get the correct coordinates
//	if (worldSpace.w != 0.0f) {
//		worldSpace /= worldSpace.w;
//	}
//
//	return glm::vec3(worldSpace);
//}

void GetRayFromMouse(float mouseX, float mouseY) {
	// Retrieve the viewport dimensions
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glm::ivec2 viewSize(viewport[2], viewport[3]);

	// The ray's origin is the camera's position
	//rayOrigin = Unproject(mouseX, mouseY, viewSize);
	//rayFar = Unproject(mouseX, mouseY, viewSize, 10.f);
	//test(mouseX, mouseY, viewSize);
	rayOrigin = cameraPos;

	// Get the ray's direction from the mouse position
	rayDir = GetRayDirection(mouseX, mouseY, viewSize);
}

// called when the mouse button transitions down or up:
void
MouseButton( int button, int state, int x, int y )
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if( DebugOn != 0 )
		fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );

	// get the proper button bit mask:
	switch( button )
	{
		case GLUT_LEFT_BUTTON:
			if (DebugOn != 0)
				fprintf(stderr, "We thinks its Left: %d\n", button);
			b = LEFT;		break;

		case GLUT_MIDDLE_BUTTON:
			if (DebugOn != 0)
				fprintf(stderr, "We thinks its Middle: %d\n", button);
			b = MIDDLE;		break;

		case GLUT_RIGHT_BUTTON:
			if (DebugOn != 0)
				fprintf(stderr, "We thinks its Right: %d\n", button);
			b = RIGHT;		break;

		case SCROLL_WHEEL_UP:
			if (DebugOn != 0)
				fprintf(stderr, "We thinks its wheel up: %d\n", button);
		//	Scale += SCLFACT * zoomSpeed;
		//	// keep object from turning inside-out or disappearing:
		//	if (Scale < minFov)
		//		fov = minFov;
			break;

		case SCROLL_WHEEL_DOWN:
			if (DebugOn != 0)
				fprintf(stderr, "We thinks its wheel down: %d\n", button);
		//	fov -= SCLFACT * zoomSpeed;
		//	// keep object from turning inside-out or disappearing:
		//	if (fov < minFov)
		//		fov = minFov;
			break;

		default:
			b = 0;
			fprintf( stderr, "Unknown mouse button: %d\n", button );
	}

	int modifiers = glutGetModifiers();
	bool shiftPressed = (modifiers & GLUT_ACTIVE_SHIFT) != 0;

	// button down sets the bit, up clears the bit:
	if(state == GLUT_DOWN)
	{
		if (button == GLUT_LEFT_BUTTON && !shiftPressed)
		{
			GetRayFromMouse(x, y);
			chess.HandleSelection(rayOrigin, rayDir);
		}
		else
		{
			if (button == GLUT_LEFT_BUTTON && shiftPressed)
			{
				if (DebugOn != 0)
					fprintf(stderr, "Shift be down and Left Click triggered!");
				// Handle camera movement

				ActiveButton |= b; // set the proper bit for camera movement
			}
			else if (button == SCROLL_WHEEL_UP)
			{
				Scale -= SCLFACT * zoomSpeed; // Decrease the FOV (zoom in)
				if (Scale < minFov)
					Scale = minFov;
			}
			else if (button == SCROLL_WHEEL_DOWN)
			{
				Scale += SCLFACT * zoomSpeed; // Increase the FOV (zoom out)
				if (Scale > maxFov)
					Scale = maxFov;
			}

			// Recalculate projection matrix with the new FOV:
			GLsizei vx = glutGet(GLUT_WINDOW_WIDTH);
			GLsizei vy = glutGet(GLUT_WINDOW_HEIGHT);
			aspectRatio = (float)vx / (float)vy;

			UpdateCamera();
		}

		Xmouse = x; // new current position
		Ymouse = y;
	}
	else
	{
		ActiveButton &= ~b; // clear the proper bit
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();

}

// called when the mouse moves while a button is down:
void
MouseMotion( int x, int y )
{
	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	// TODO have on hover to highlight the piece

	if( ( ActiveButton & LEFT ) != 0 )
	{
		Xrot += ( ANGFACT*dy );
		Yrot += ( ANGFACT*dx );
	}

	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );

		// keep object from turning inside-out or disappearing:

		if(Scale < minFov )
			Scale = minFov;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	UpdateCamera();

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}
#pragma endregion

// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene
void
Reset( )
{
	ActiveButton = 0;
	AxesOn = 0;
	DebugOn = 1;
	DepthBufferOn = 1;
	DepthFightingOn = 0;
	DepthCueOn = 0;
	Scale = .05;
	ShadowsOn = 0;
	NowColor = YELLOW;
	NowProjection = PERSP;
	Xrot = Yrot = 0.;
	LightMode = false;
	TextureType = true;
	//cameraPos = glm::vec3(400, 400, 400); // Camera position
	//cameraTarget = glm::vec3(0, 0, 0); // Look-at point
	//upVector = glm::vec3(0, 1, 0);     // Up vector
	//float fov = 1.f;                          // Field of view
	rayDir = glm::vec3(69);
	rayOrigin = glm::vec3(69);
}

// called when user resizes the window:
void
Resize( int width, int height )
{
	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

// handle a change to the window's visibility:
void
Visibility ( int state )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE )
	{
		glutSetWindow( MainWindow );
		glutPostRedisplay( );
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}

///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////

#pragma region  the stroke characters 'X' 'Y' 'Z' :

static float xx[ ] = { 0.f, 1.f, 0.f, 1.f };

static float xy[ ] = { -.5f, .5f, .5f, -.5f };

static int xorder[ ] = { 1, 2, -3, 4 };

static float yx[ ] = { 0.f, 0.f, -.5f, .5f };

static float yy[ ] = { 0.f, .6f, 1.f, 1.f };

static int yorder[ ] = { 1, 2, 3, -2, 4 };

static float zx[ ] = { 1.f, 0.f, 1.f, 0.f, .25f, .75f };

static float zy[ ] = { .5f, .5f, -.5f, -.5f, 0.f, 0.f };

static int zorder[ ] = { 1, 2, 3, 4, -5, 6 };

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

#pragma endregion

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)
void
Axes( float length )
{
	glBegin( GL_LINE_STRIP );
		glVertex3f( length, 0., 0. );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., length, 0. );
	glEnd( );
	glBegin( GL_LINE_STRIP );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., 0., length );
	glEnd( );

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 4; i++ )
		{
			int j = xorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 5; i++ )
		{
			int j = yorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 6; i++ )
		{
			int j = zorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
		}
	glEnd( );

}

char* ToChar(const char* crap)
{
	// Get the length of the const char*
	size_t length = std::strlen(crap);

	// Create a char* buffer of the same length
	char* charBuffer = new char[length + 1]; // +1 for the null terminator

	// Copy the content from const char* to char*
	std::strcpy(charBuffer, crap);

	return charBuffer;
}

#pragma region CustomMathMethods
	// function to convert HSV to RGB
	// 0.  <=  s, v, r, g, b  <=  1.
	// 0.  <= h  <=  360.
	// when this returns, call:
	//		glColor3fv( rgb );

	void
	HsvRgb( float hsv[3], float rgb[3] )
	{
		// guarantee valid input:

		float h = hsv[0] / 60.f;
		while( h >= 6. )	h -= 6.;
		while( h <  0. ) 	h += 6.;

		float s = hsv[1];
		if( s < 0. )
			s = 0.;
		if( s > 1. )
			s = 1.;

		float v = hsv[2];
		if( v < 0. )
			v = 0.;
		if( v > 1. )
			v = 1.;

		// if sat==0, then is a gray:

		if( s == 0.0 )
		{
			rgb[0] = rgb[1] = rgb[2] = v;
			return;
		}

		// get an rgb from the hue itself:
	
		float i = (float)floor( h );
		float f = h - i;
		float p = v * ( 1.f - s );
		float q = v * ( 1.f - s*f );
		float t = v * ( 1.f - ( s * (1.f-f) ) );

		float r=0., g=0., b=0.;			// red, green, blue
		switch( (int) i )
		{
			case 0:
				r = v;	g = t;	b = p;
				break;
	
			case 1:
				r = q;	g = v;	b = p;
				break;
	
			case 2:
				r = p;	g = v;	b = t;
				break;
	
			case 3:
				r = p;	g = q;	b = v;
				break;
	
			case 4:
				r = t;	g = p;	b = v;
				break;
	
			case 5:
				r = v;	g = p;	b = q;
				break;
		}


		rgb[0] = r;
		rgb[1] = g;
		rgb[2] = b;
	}

	void
	Cross(float v1[3], float v2[3], float vout[3])
	{
		float tmp[3]{};
		tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
		tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
		tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];
		vout[0] = tmp[0];
		vout[1] = tmp[1];
		vout[2] = tmp[2];
	}

	float
	Dot(float v1[3], float v2[3])
	{
		return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
	}


	float
	Unit(float vin[3], float vout[3])
	{
		float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];
		if (dist > 0.0)
		{
			dist = sqrtf(dist);
			vout[0] = vin[0] / dist;
			vout[1] = vin[1] / dist;
			vout[2] = vin[2] / dist;
		}
		else
		{
			vout[0] = vin[0];
			vout[1] = vin[1];
			vout[2] = vin[2];
		}
		return dist;
	}


	float
	Unit( float v[3] )
	{
		float dist = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
		if (dist > 0.0)
		{
			dist = sqrtf(dist);
			v[0] /= dist;
			v[1] /= dist;
			v[2] /= dist;
		}
		return dist;
	}

	bool AreVec3Equal(const glm::vec3& vec1, const glm::vec3& vec2, float tolerance) {
		return (std::abs(vec1.x - vec2.x) < tolerance) &&
			(std::abs(vec1.y - vec2.y) < tolerance) &&
			(std::abs(vec1.z - vec2.z) < tolerance);
	}
#pragma endregion