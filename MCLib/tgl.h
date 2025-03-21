//-------------------------------------------------------------------------------
//
// Tiny Geometry Layer
//
// For MechCommander 2 -- Copyright (c) 1999 Microsoft
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef TGL_H
#define TGL_H
//-------------------------------------------------------------------------------
// Include Files
#ifndef HEAP_H
#include "heap.h"
#endif

#ifndef FILE_H
#include "file.h"
#endif

#include <stuff\stuff.hpp>
#include <gameos.hpp>

//-------------------------------------------------------------------------------
// Structs used by layer.
//
typedef DWORD* DWORDPtr;

//-------------------------------------------------------------------------------
// TG_TypeVertex
//This Structure stores information for each vertex of the shape which DOES NOT CHANGE by instance
typedef struct _TG_TypeVertex
{
	//Only changes at load time.
	Stuff::Point3D	position;				//Position of vertex relative to base position of shape.
	Stuff::Vector3D normal;					//Vertex Normal
	DWORD    		aRGBLight;				//Vertex Light and Alpha

} TG_TypeVertex;

typedef  TG_TypeVertex* TG_TypeVertexPtr;

//-------------------------------------------------------------------------------
// TG_Vertex
// This Structure stores information for each vertex of the shape which is instance specific
// Its used to store UNCHANGING or rarely changing Light/Fog data so I don't have to calc every frame!
typedef struct _TG_Vertex
{
	BYTE			fog;
	BYTE			redSpec;
	BYTE			greenSpec;
	BYTE			blueSpec;
} TG_Vertex;

typedef  TG_Vertex* TG_VertexPtr;

//-------------------------------------------------------------------------------
// TG_ShadowVertex
//This Structure stores information for each vertex of the shadow for this shape.
// Nothing changes frame to frame here anymore unless light source is moving which is now RARE!
typedef struct _TG_ShadowVertex
{
	//Changes every frame if local light.  Rarely for the sun.  Moves slowly!
	bool 			bDataIsNotValid;		//Indicates that the struct contains unitialized data
	Stuff::Point3D	position;				//Position of vertex relative to base position of shape.
} TG_ShadowVertex;

typedef  TG_ShadowVertex* TG_ShadowVertexPtr;

//-----------------------------------------------------------------------------------
// TG_ShadowVertexTemp
// Stores VOLATILE information for the shadow vertices.  These can come from a pool!
typedef struct _TG_ShadowVertexTemp
{
	DWORD    		fRGBFog;				//Vertex Fog and Specular Color.  Needed if shadow is fogged.
											//Every frame for local light.  Once in a blue moon for infinite light.

	//Changes each call to Transform.
	Stuff::Vector4D transformedPosition;	//ScreenCoords with Z depth.  Valid after Transform called.
} TG_ShadowVertexTemp;

typedef  TG_ShadowVertexTemp* TG_ShadowVertexTempPtr;

//-------------------------------------------------------------------------------
// TG_UVData
typedef struct _TG_UVData
{
	float			u0;						//UV Data for each vertex of the triangle
	float 			v0;						//Allows same vertex to have different UVs
	float			u1;
	float			v1;
	float			u2;
	float			v2;

} TG_UVData;

typedef  TG_UVData* TG_UVDataPtr;

//-------------------------------------------------------------------------------
// TG_TypeTriangle
// This structure stores the information needed to draw the triangle which does NOT change per instance
typedef struct _TG_TypeTriangle
{
	DWORD    		Vertices[3];			//Indices into Vertex List.
	DWORD    		localTextureHandle;		//Index into texture List.
	DWORD    		renderStateFlags;		//Flags about render for this face.
											//Bit 0 -- backFacing
	TG_UVData  		uvdata;					//Texture UVs for this face.
	Stuff::Vector3D faceNormal;				//Normal Vector to face
} TG_TypeTriangle;

// Texture handle can change from frame to frame. Texture Animation!

typedef TG_TypeTriangle* TG_TypeTrianglePtr;

//-------------------------------------------------------------------------------
// TG_Triangle
// This structure stores the information needed to draw the triangle per instance.
typedef struct _TG_Triangle
{
	DWORD    		aRGBLight[3];			//RGB Light for this triangle's vertices.
	DWORD    		fRGBLight[3];			//RGB Fog for this triangle's vertices.
} TG_Triangle;

// Texture handle can change from frame to frame. Texture Animation!

typedef TG_Triangle* TG_TrianglePtr;

//-------------------------------------------------------------------------------
// TG_ShadowTriangle
// This structure stores the information needed to draw the shadow triangle.
typedef struct _TG_ShadowTriangle
{
	DWORD    		Vertices[3];			//Indices into Shadow Vertex List.
} TG_ShadowTriangle;

// Texture handle can change from frame to frame. Texture Animation!

typedef TG_ShadowTriangle* TG_ShadowTrianglePtr;

//-------------------------------------------------------------------------------
// TG_LightTypes
#define 	TG_LIGHT_NONE					0xffffffff
#define 	TG_LIGHT_AMBIENT				0
#define		TG_LIGHT_INFINITE				1
#define		TG_LIGHT_INFINITEWITHFALLOFF	2
#define 	TG_LIGHT_POINT					3
#define		TG_LIGHT_SPOT					4
#define		TG_LIGHT_TERRAIN				5

#define 	MAX_LIGHTS_IN_WORLD				256

#define 	TG_NODE_ID						25

#define		MAX_SCAN_LENGTH					40

#define 	MAX_NODES						256

#define		MAX_SHADOWS						1
//-------------------------------------------------------------------------------
// TG_Light
// This structure stores the information necessary to light the shape.
typedef struct _TG_Light
{
	DWORD    				lightType;			//Ambient, directional, etc.
	bool					active;				//Should this light be considered on?
	
	protected:
	DWORD    				aRGB;				//Color
	DWORD					OEMaRGB;
	
	public:
	float					intensity;			//How Bright
	float    				closeDistance;		//Distance out light is constant
	float    				farDistance;		//Distance at which light is off
	float					oneOverDistance;	//Used for falloff calc
	Stuff::Point3D 			direction;			//Direction in world of light source.  This is the light spot center for POINT and SPOT
	Stuff::LinearMatrix4D	lightToWorld;		//Transformation Matrix
	Stuff::Vector3D			position;			//Explicit position to aid terrain code with point sources.
	Stuff::Point3D			spotDir;			//Direction of the actual Spotlight to help with shadows, etc.
	float					maxSpotLength;		//Maximum length spotlight can be from target.

	void init (DWORD lType)
	{
		gosASSERT((lType != TG_LIGHT_NONE) && (lType >= TG_LIGHT_AMBIENT) && (lType <= TG_LIGHT_TERRAIN));

		lightType = lType;
		aRGB = OEMaRGB = 0xffffffff;
		intensity = 1.0f;
		direction.x = direction.y = direction.z = 0.0f;
		closeDistance = 0.0f;
		farDistance = 0.0f;
		oneOverDistance = 0.0f;
		position.x = position.y = position.z = -999999.0f;
		spotDir.x = spotDir.y = spotDir.z = 0.0f;
		maxSpotLength = 0.0f;
		active = false;
	}

	void SetaRGB (DWORD newaRGB)
	{
		OEMaRGB = aRGB = newaRGB;
	}

	DWORD GetaRGB (void)
	{
		return aRGB;
	}
	
	void SetLightToWorld (Stuff::LinearMatrix4D *l2w)
	{
		lightToWorld = *l2w;
	}

	void SetPosition (Stuff::Vector3D *pos)
	{
		position = *pos;
	}

	void SetFalloffDistances (float inner, float outer)
	{
		closeDistance = inner;
		farDistance = outer;

		oneOverDistance = 1.0f/(outer - inner);
	}

	void SetIntensity (float intensity)
	{
		if (intensity > 1.0f)
			intensity = 1.0f;

		//Scale aRGB by intensity
		DWORD r,g,b;
		r = intensity * ((OEMaRGB >> 16) & 0x000000ff);
		g = intensity * ((OEMaRGB >> 8) & 0x000000ff);
		b = intensity * ((OEMaRGB) & 0x000000ff);

		aRGB = (0xff << 24) + (r << 16) + (g << 8) + (b);
	}

	bool GetFalloff(float length, float &falloff)
	{
		if (length <= closeDistance)
		{
			falloff = 1.0f;
			return true;
		}

		if (length >= farDistance)
		{
			return false;
		}

		falloff = (farDistance - length) * oneOverDistance;
		return true;
	}

} TG_Light;

typedef  TG_Light *TG_LightPtr;

//-------------------------------------------------------------------------------
// TG_Texture
typedef struct _TG_Texture
{
	char				textureName[256];
	DWORD				mcTextureNodeIndex;
	DWORD				gosTextureHandle;
	bool				textureAlpha;
} TG_Texture;

typedef TG_Texture* TG_TexturePtr;

//-------------------------------------------------------------------------------
// TG_TinyTexture
typedef struct _TG_TinyTexture
{
	DWORD				mcTextureNodeIndex;
	DWORD				gosTextureHandle;
	bool				textureAlpha;
} TG_TinyTexture;

typedef TG_TinyTexture* TG_TinyTexturePtr;

class TG_Shape;
//-------------------------------------------------------------------------------
// TG_Animation
typedef struct _TG_Animation
{
	char 						nodeId[TG_NODE_ID];		//Node ID
	DWORD						shapeId;				//DON'T SCAN EVERY FRAME.  WOW IS IT SLOW!!!!!  Set this first time through and its simple.
	DWORD						numFrames;				//Number of Frames of animation.
	float 						frameRate;				//Number of Frames Per Second.
	float						tickRate;				//Number of Ticks Per Second.
	Stuff::UnitQuaternion		*quat;					//Stores animation offset in Quaternion rotation.
	Stuff::Point3D				*pos;					//Stores Positional offsets if present.  OTHERWISE NULL!!!!!!!!
	
	void SaveBinaryCopy (File *binFile);
	void LoadBinaryCopy (File *binFile);
	
} TG_Animation;

typedef TG_Animation *TG_AnimationPtr;

//-------------------------------------------------------------------------------
// TG_ShapeRec
typedef struct _TG_ShapeRec
{
	TG_Shape*					node;					//Pointer to the actual TG_Shape for this piece
	Stuff::LinearMatrix4D		localShapeToWorld;		//Matrix to transform this TG_Shape.
	Stuff::LinearMatrix4D		shapeToWorld;			//Matrix to transform this TG_Shape.
	Stuff::LinearMatrix4D		worldToShape;			//Inverse of above Matrix.
	long						calcedThisFrame;		//Turn number this matrix is current for.
	bool						processMe;				//Flag indicating if I should transform/draw this.  Used for arms off.
	TG_AnimationPtr				currentAnimation;		//Animation data being applied to this shape.  OK if NULL
	_TG_ShapeRec				*parentNode;			//Parent Node.  OK if NULL but only for ROOT node!
	Stuff::UnitQuaternion		baseRotation;			//Always ZERO unless set by appearance controlling this shape.
	
	_TG_ShapeRec &operator=(const _TG_ShapeRec &rec)
	{
		node 				= rec.node;
		calcedThisFrame		= -1;
		processMe			= true;
		currentAnimation	= NULL;
		parentNode			= rec.parentNode;
		baseRotation		= rec.baseRotation;

		return *this;
	}
	
} TG_ShapeRec;

typedef TG_ShapeRec *TG_ShapeRecPtr;

#define TYPE_NODE			0
#define SHAPE_NODE			1

class TG_TypeNode
{
	//---------------
	// Data Members
	protected:
		Stuff::Point3D			nodeCenter;
		Stuff::Point3D			relativeNodeCenter;
		char					nodeId[TG_NODE_ID];
		char					parentId[TG_NODE_ID];
		
	//---------------------
	// Member Functions
	protected:

	public:
		void *operator new (size_t mySize);
		void operator delete (void *us);
			
 		virtual void init (void)
		{
			nodeId[0] = parentId[0] = '\0';
			relativeNodeCenter.x = relativeNodeCenter.y = relativeNodeCenter.z = 0.0f;
			nodeCenter.x = nodeCenter.y = nodeCenter.z = 0.0f;
		}
		
		virtual void destroy (void);
		
		char *getNodeId (void)
		{
			return nodeId;
		}

		char *getParentId (void)
		{
			return parentId;
		}

		Stuff::Point3D GetNodeCenter (void)
		{
			return nodeCenter;
		}

		Stuff::Point3D GetRelativeNodeCenter (void)
		{
			return relativeNodeCenter;
		}

		void MoveNodeCenterRelative (Stuff::Point3D parent)
		{
			relativeNodeCenter = nodeCenter;
			relativeNodeCenter -= parent;
		}
		
		virtual void movePosRelativeCenterNode (void)
		{
		}

		virtual long GetNumTypeVertices (void)
		{
			return 0;
		}

		virtual long GetNodeType (void)
		{
			return TYPE_NODE;
		}
		
		//Function return 0 is OK.  -1 if file is not ASE Format or missing data.
		//This function simply parses the ASE buffers handed to it.  This allows
		//users to load the ase file themselves and manage their own memory for it.
		//It allocates memory for internal Lists.  These are straight mallocs at present.
		//
		// NOTE: Only takes the first GEOMOBJECT from the ASE file.  Multi-object
		// Files will require user intervention to parse!!
		virtual long ParseASEFile (BYTE *aseBuffer, char *filename)
		{
			return 0;
		}

		//Function return 0 is OK.  -1 if file is not ASE Format or missing data.
		//This function simply parses the ASE buffers handed to it.  This allows
		//users to load the ase file themselves and manage their own memory for it.
		//It allocates memory for internal Lists.  These are straight mallocs at present.
		//
		// NOTE: Only takes the first HELPEROBJECT from the ASE file.  Multi-object
		// Files will require user intervention to parse!!
		virtual long MakeFromHelper (BYTE *aseBuffer, char *filename);

		//Function returns 0 if OK.  -1 if file not found or file not ASE Format.		
		//This function loads the ASE file into the TG_Triangle and TG_Vertex lists.
		//It allocates memory.  These are straight mallocs at present.
		//
		// NOTE: Only takes the first GEOMOBJECT from the ASE file.  Multi-object
		// Files will require user intervention to parse!!
		virtual long LoadTGShapeFromASE (char *fileName)
		{
			return 0;
		}

		//Function returns 0 if OK.  -1 if textureNum is out of range of numTextures.
		//This function takes the gosTextureHandle passed in and assigns it to the
		//textureNum entry of the listOfTextures;
		virtual long SetTextureHandle (DWORD textureNum, DWORD gosTextureHandle)
		{
			return 0;
		}

		//Function returns 0 if OK.  -1 if textureNum is out of range of numTextures.
		//This function takes the gosTextureHandle passed in and assigns it to the
		//textureNum entry of the listOfTextures;
		virtual long SetTextureAlpha (DWORD textureNum, bool alphaFlag)
		{
			return 0;
		}

		//Need this so that Multi-Shapes can let each shape know texture info.
		virtual void CreateListOfTextures (TG_TexturePtr list, DWORD numTxms)
		{
		}

		//--------------------------------------------------------------
		// Creates an instance of this shape for the game to muck with.
		virtual TG_Shape *CreateFrom (void);

		virtual void SetAlphaTest (bool flag)
		{
		}

		virtual void SetFilter (bool flag)
		{
		}

		virtual void SetLightRGBs (DWORD hPink, DWORD hGreen, DWORD hYellow)
		{
		}
		
 		virtual void LoadBinaryCopy (File &binFile);
		virtual void SaveBinaryCopy (File &binFile);

};

typedef TG_TypeNode* TG_TypeNodePtr;

class TG_MultiShape;
class TG_TypeMultiShape;
class TG_Shape;
//-------------------------------------------------------------------------------
// The meat and Potatoes part.
// TG_Shape
class TG_TypeShape : public TG_TypeNode
{
	//-------------------------------------------------------
	// This class runs the shape.  Knows how to load/import
	// an ASE file.  Can dig information out of the file for
	// User use (i.e. Texture Names).  Transforms, lights, clips
	// and renders the shape.

	friend TG_TypeMultiShape;
	friend TG_MultiShape;
	friend TG_Shape;
	
	//-------------
	//Data Members
	protected:
		DWORD					numTypeVertices;			//Number of vertices in Shape
		DWORD					numTypeTriangles;			//NUmber of triangles in Shape
		DWORD					numTextures;				//Number of textures in Shape

		TG_TypeVertexPtr		listOfTypeVertices;			//Memory holding all vertex data
		TG_TypeTrianglePtr		listOfTypeTriangles;		//Memory holding all triangle data
		TG_TinyTexturePtr		listOfTextures;				//List of texture Structures for this shape.

		DWORD					hotPinkRGB;					//Stores the value for this shape to replace hot Pink With
		DWORD					hotYellowRGB;				//Stores the value for this shape to replace hot Yellow With 
		DWORD					hotGreenRGB;            	//Stores the value for this shape to replace hot Green With 

		bool					alphaTestOn;				//Decides if we should draw alphaTest On or not!
		bool					filterOn;					//Decides if we should filter the shape or not!

	//-----------------
	//Member Functions
	protected:

	public:
		virtual void init (void)
		{
			numTypeVertices = numTypeTriangles = numTextures = 0;

			listOfTypeVertices = NULL;
			listOfTypeTriangles = NULL;
			listOfTextures = NULL;

			alphaTestOn = false;

			filterOn = true;

			relativeNodeCenter.x = relativeNodeCenter.y = relativeNodeCenter.z = 0.0f;
			nodeCenter.x = nodeCenter.y = nodeCenter.z = 0.0f;
			
			hotPinkRGB = 0x00cbf0ff;
			hotYellowRGB = 0x00FEfF91;
			hotGreenRGB = 0x000081b6;
		}
		
		TG_TypeShape (void)
		{
			init();
		}

		virtual void destroy (void);

		~TG_TypeShape (void)
		{
			destroy();
		}

		virtual long GetNumTypeVertices (void)
		{
			return numTypeVertices;
		}
		
 		//Function return 0 is OK.  -1 if file is not ASE Format or missing data.
		//This function simply parses the ASE buffers handed to it.  This allows
		//users to load the ase file themselves and manage their own memory for it.
		//It allocates memory for internal Lists.  These are straight mallocs at present.
		//
		// NOTE: Only takes the first GEOMOBJECT from the ASE file.  Multi-object
		// Files will require user intervention to parse!!
		virtual long ParseASEFile (BYTE *aseBuffer, char *filename);	//filename for error reporting ONLY

		//Function return 0 is OK.  -1 if file is not ASE Format or missing data.
		//This function simply parses the ASE buffers handed to it.  This allows
		//users to load the ase file themselves and manage their own memory for it.
		//It allocates memory for internal Lists.  These are straight mallocs at present.
		//
		// NOTE: Only takes the first HELPEROBJECT from the ASE file.  Multi-object
		// Files will require user intervention to parse!!
		virtual long MakeFromHelper (BYTE *aseBuffer, char *filename);

		//Function returns 0 if OK.  -1 if file not found or file not ASE Format.		
		//This function loads the ASE file into the TG_Triangle and TG_Vertex lists.
		//It allocates memory.  These are straight mallocs at present.
		//
		// NOTE: Only takes the first GEOMOBJECT from the ASE file.  Multi-object
		// Files will require user intervention to parse!!
		virtual long LoadTGShapeFromASE (char *fileName);

		//Function returns 0 if OK.  -1 if textureNum is out of range of numTextures.
		//This function takes the gosTextureHandle passed in and assigns it to the
		//textureNum entry of the listOfTextures;
		virtual long SetTextureHandle (DWORD textureNum, DWORD gosTextureHandle);

		//Function returns 0 if OK.  -1 if textureNum is out of range of numTextures.
		//This function takes the gosTextureHandle passed in and assigns it to the
		//textureNum entry of the listOfTextures;
		virtual long SetTextureAlpha (DWORD textureNum, bool alphaFlag);

		//Need this so that Multi-Shapes can let each shape know texture info.
		virtual void CreateListOfTextures (TG_TexturePtr list, DWORD numTxms);

 		virtual void movePosRelativeCenterNode (void);
		
		//--------------------------------------------------------------
		// Creates an instance of this shape for the game to muck with.
		virtual TG_Shape *CreateFrom (void);

		virtual void SetAlphaTest (bool flag)
		{
			alphaTestOn = flag;
		}

		virtual void SetFilter (bool flag)
		{
			filterOn = flag;
		}

		virtual void SetLightRGBs (DWORD hPink, DWORD hGreen, DWORD hYellow)
		{
			hotPinkRGB = hPink;
			hotGreenRGB = hGreen;
			hotYellowRGB = hYellow;
		}
		
		virtual long GetNodeType (void)
		{
			return SHAPE_NODE;
		}
		
 		virtual void LoadBinaryCopy (File &binFile);
		virtual void SaveBinaryCopy (File &binFile);
};

typedef TG_TypeShape* TG_TypeShapePtr;

//-------------------------------------------------------------------------------------
// The OTHER meat and Potatoes part.  This is the actual instance the game draws with.
// TG_Shape
class TG_Shape
{
	//-------------------------------------------------------
	// This class runs the shape instance.
	// Transforms, lights, clips and renders the shape.

	friend TG_MultiShape;
	friend TG_TypeShape;
	friend TG_TypeNode;
	friend TG_TypeMultiShape;
	
	//-------------
	//Data Members
	protected:
		TG_TypeNodePtr			myType;						//Pointer to the instance of the shape.
		DWORD					numVertices;				//Number of vertices in Shape
		DWORD					numTriangles;				//NUmber of triangles in Shape
		DWORD					numVisibleFaces;			//Number of non-backfaced non-clipped faces.
		DWORD					numVisibleShadows;			//Number of visible Shadow Faces.

		TG_Vertex *				listOfColors;				//Memory holding all unchanged or rarely changed color data.
		gos_VERTEX * 			listOfVertices;				//Memory holding all vertex data
		TG_TrianglePtr			listOfTriangles;			//Memory holding all triangle data
		DWORDPtr				listOfVisibleFaces;			//Memory holding indices into listOfTriangles
															//Draw in this order.  First entry with value 0xffffffff
															//Means all done, no more to draw.

		TG_ShadowVertexPtr		listOfShadowVertices;		//Stores shadow vertex information for the shape.
															//We just use existing listOfTriangles to draw!
															
		TG_ShadowVertexTempPtr	listOfShadowTVertices;		//Stores just the Volatile data.  Comes from a pool!!
															
		DWORDPtr				listOfVisibleShadows;		//Memory holding indices into listOfTriangles
															//Draw in this order.  First entry with value 0xffffffff
															//Means all done, no more to draw.

		bool					shadowsVisible[MAX_SHADOWS];//Is this shadow worth drawing?

		DWORD					aRGBHighlight;				//Color to add to vertices to make building stand out.
		DWORD					fogRGB;						//Color to make fog.
		
		float					shapeScalar;

		bool					lightsOut;
		bool					noShadow;
		bool					recalcShadows;
		bool					isWindow;
		bool					isSpotlight;

		DWORD					lastTurnTransformed;

	public:
		//Matrices used to transform the shapes.
		static Stuff::LinearMatrix4D 	*cameraOrigin;
		static Stuff::Matrix4D			*cameraToClip;
		static Stuff::Matrix4D			worldToClip;
		static Stuff::LinearMatrix4D	worldToCamera;
		static TG_LightPtr				*listOfLights;		//List passed in a transform time
		static DWORD					numLights;

		static float					viewMulX;
		static float					viewAddX;
		static float					viewMulY;
		static float					viewAddY;

		static DWORD					fogColor;
		static float					fogFull;
		static float					fogStart;

		static Stuff::LinearMatrix4D 	lightToShape[MAX_LIGHTS_IN_WORLD];
		static Stuff::Vector3D			lightDir[MAX_LIGHTS_IN_WORLD];
		static Stuff::Vector3D			spotDir[MAX_LIGHTS_IN_WORLD];
		static Stuff::Vector3D			rootLightDir[MAX_LIGHTS_IN_WORLD];

		static UserHeapPtr 				tglHeap;		//Stores all TGL data so we don't need to go through the FREE merry go round of GOS!
		
		static DWORD					lighteningLevel;

	//-----------------
	//Member Functions
	protected:

	public:
		void * operator new (size_t mySize);
		void operator delete (void * us);

		void init (void)
		{
			numVertices = numTriangles = numVisibleFaces = 0;

			listOfVertices = NULL;
			listOfTriangles = NULL;
			listOfLights = NULL;
			listOfVisibleFaces = NULL;

			listOfShadowVertices = NULL;
			listOfVisibleShadows = NULL;

			aRGBHighlight = 0x00000000;
			
			shapeScalar = 0.0f;
			
			lightsOut = false;
			
			noShadow = false;
			
			recalcShadows = true;

			isWindow = isSpotlight = false;

			for (long i=0;i<MAX_SHADOWS;i++)
				shadowsVisible[i] = false;
		}
		
		TG_Shape (void)
		{
			init();
		}

		void destroy (void);

		~TG_Shape (void)
		{
			destroy();
		}

		//This function sets up the camera Matrices for this TG_Shape to transform
		//itself with.  These matrices are static and only need to be set once per
		//render pass if the camera does not change for that pass.
		static void SetCameraMatrices (Stuff::LinearMatrix4D *camOrigin, Stuff::Matrix4D *camToClip);

		static void SetViewport (float mulX, float mulY, float addX, float addY);

		static void SetFog (DWORD fRGB, float fStart, float fFull);

		//This function sets the list of lights used by the TransformShape function
		//to light the shape.
		//Function returns 0 if lightList entries are all OK.  -1 otherwise.
		//
		long SetLightList (TG_LightPtr *lightList, DWORD nLights);
		
		//This function sets the fog values for the shape.  Straight fog right now.
		void SetFogRGB (DWORD fRGB);

		//This function does exactly what TranformShape does EXCEPT that the shapeToClip,
		//Lighting and backface matrices have been calculated in the step above this one.
		//This saves enormous processor cycles when matrices are the same and transforms
		//Can just be run from the same matrices without recalcing them!
		//Function returns -1 if all vertex screen positions are off screen.
		//Function returns 0 if any one vertex screen position is off screen.
		//Function returns 1 is all vertex screen positions are on screen.
		// NOTE:  THIS IS NOT A RIGOROUS CLIP!!!!!!!!!
		long MultiTransformShape (Stuff::Matrix4D *shapeToClip, Stuff::Point3D *backFacePoint, TG_ShapeRecPtr parentNode, bool isHudElement, BYTE alphaValue, bool isClamped);

		//This function creates the list of shadows and transforms them in preparation to drawing.
		//
		void MultiTransformShadows (Stuff::Point3D *pos, Stuff::LinearMatrix4D *s2w, float rotation);

		//This function takes the current listOfVisibleFaces and draws them using
		//gos_DrawTriangle.  Does clipping, too!
		void Render (float forceZ = -1.0f, bool isHudElement = false, BYTE alphaValue = 0xff, bool isClamped = false);

		//This function takes the current listOfShadowTriangles and draws them using
		//gos_DrawTriangle.  Does clipping, too!
		long RenderShadows (long startFace);

		void SetARGBHighLight (DWORD argb)
		{
			aRGBHighlight = argb;
		}

		void SetLightsOut (bool lightFlag)
		{
			lightsOut = lightFlag;
		}
		
		char * getNodeName (void)
		{
			return myType->getNodeId();
		}
		
		Stuff::Point3D GetRelativeNodeCenter (void)
		{
			return myType->GetRelativeNodeCenter();
		}
		
		bool PerPolySelect (float mouseX, float mouseY);
		
		void SetRecalcShadows (bool flag)
		{
			recalcShadows = flag;
		}
		
		virtual void ScaleShape (float scaleFactor)
		{
			shapeScalar = scaleFactor;
		}
};

typedef TG_Shape* TG_ShapePtr;

//Pools are defined here.

class TG_VertexPool
{
	protected:
		TG_Vertex 	*tgVertexPool;
		TG_Vertex 	*nextVertex;
		
		DWORD		totalVertices;
		DWORD		numVertices;
		
	public:
		TG_VertexPool (void)
		{
			tgVertexPool = nextVertex = NULL;
			totalVertices = numVertices = 0;
		}
		
		~TG_VertexPool (void)
		{
			destroy();
		}
		
		void destroy (void)
		{
			TG_Shape::tglHeap->Free(tgVertexPool);
			tgVertexPool = nextVertex = NULL;
			totalVertices = numVertices = 0;
		}
		
		void init (DWORD maxVertices)
		{
			tgVertexPool = (TG_VertexPtr)TG_Shape::tglHeap->Malloc(sizeof(TG_Vertex) * maxVertices);
			gosASSERT(tgVertexPool != NULL);
			
			nextVertex = tgVertexPool;
			
			totalVertices = maxVertices;
			numVertices = 0;
		}
		
		void reset (void)
		{
			nextVertex = tgVertexPool;
			numVertices = 0;
		}
		
		TG_VertexPtr getColorsFromPool (DWORD numRequested)
		{
			TG_VertexPtr result = NULL;
			numVertices += numRequested;
			if (numVertices < totalVertices)
			{
				result = nextVertex;
				nextVertex += numRequested;
			}
			
			return result;
		}
};

class TG_GOSVertexPool
{
	protected:
		gos_VERTEX 	*gVertexPool;
		gos_VERTEX 	*nextVertex;
		
		DWORD		totalVertices;
		DWORD		numVertices;
		
	public:
		TG_GOSVertexPool (void)
		{
			gVertexPool = nextVertex = NULL;
			totalVertices = numVertices = 0;
		}
		
		~TG_GOSVertexPool (void)
		{
			destroy();
		}
		
		void destroy (void)
		{
			TG_Shape::tglHeap->Free(gVertexPool);
			gVertexPool = nextVertex = NULL;
			totalVertices = numVertices = 0;
		}
		
		void init (DWORD maxVertices)
		{
			gVertexPool = (gos_VERTEX *)TG_Shape::tglHeap->Malloc(sizeof(gos_VERTEX) * maxVertices);
			gosASSERT(gVertexPool != NULL);
			
			nextVertex = gVertexPool;
			
			totalVertices = maxVertices;
			numVertices = 0;
		}
		
		void reset (void)
		{
			nextVertex = gVertexPool;
			numVertices = 0;
		}
		
		gos_VERTEX * getVerticesFromPool (DWORD numRequested)
		{
			gos_VERTEX* result = NULL;
			numVertices += numRequested;
			if (numVertices < totalVertices)
			{
				result = nextVertex;
				nextVertex += numRequested;
			}
			
			return result;
		}
};

class TG_TrianglePool
{
	protected:
		TG_Triangle *trianglePool;
		TG_Triangle	*nextTriangle;
		
		DWORD		totalTriangles;
		DWORD		numTriangles;
		
	public:
		TG_TrianglePool (void)
		{
			trianglePool = nextTriangle = NULL;
			totalTriangles = numTriangles = 0;
		}
		
		~TG_TrianglePool (void)
		{
			destroy();
		}
		
		void destroy (void)
		{
			TG_Shape::tglHeap->Free(trianglePool);
			trianglePool = nextTriangle = NULL;
			totalTriangles = numTriangles = 0;
		}
		
		void init (DWORD maxTriangles)
		{
			trianglePool = (TG_Triangle *)TG_Shape::tglHeap->Malloc(sizeof(TG_Triangle) * maxTriangles);
			gosASSERT(trianglePool != NULL);
			
			nextTriangle = trianglePool;
			
			totalTriangles = maxTriangles;
			numTriangles = 0;
		}
		
		void reset (void)
		{
			nextTriangle = trianglePool;
			numTriangles = 0;
		}
		
		TG_Triangle * getTrianglesFromPool (DWORD numRequested)
		{
			TG_Triangle* result = NULL;
			numTriangles += numRequested;
			if (numTriangles < totalTriangles)
			{
				result = nextTriangle;
				nextTriangle += numRequested;
			}
			
			return result;
		}
};

class TG_ShadowPool
{
	protected:
		TG_ShadowVertexTemp 	*tVertexPool;
		TG_ShadowVertexTemp 	*nextVertex;
		
		DWORD					totalVertices;
		DWORD					numVertices;
		
	public:
		TG_ShadowPool (void)
		{
			tVertexPool = nextVertex = NULL;
			totalVertices = numVertices = 0;
		}
		
		~TG_ShadowPool (void)
		{
			destroy();
		}
		
		void destroy (void)
		{
			TG_Shape::tglHeap->Free(tVertexPool);
			tVertexPool = nextVertex = NULL;
			totalVertices = numVertices = 0;
		}
		
		void init (DWORD maxVertices)
		{
			tVertexPool = (TG_ShadowVertexTempPtr)TG_Shape::tglHeap->Malloc(sizeof(TG_ShadowVertexTemp) * maxVertices);
			gosASSERT(tVertexPool != NULL);
			
			nextVertex = tVertexPool;
			
			totalVertices = maxVertices;
			numVertices = 0;
		}
		
		void reset (void)
		{
			nextVertex = tVertexPool;
			numVertices = 0;
		}
		
		TG_ShadowVertexTempPtr getShadowsFromPool (DWORD numRequested)
		{
			TG_ShadowVertexTempPtr result = NULL;
			numVertices += numRequested;
			if (numVertices < totalVertices)
			{
				result = nextVertex;
				nextVertex += numRequested;
			}
			
			return result;
		}
};

class TG_DWORDPool
{
	protected:
		DWORD 	*triPool;
		DWORD	*nextTri;
		
		DWORD	totalTriangles;
		DWORD	numTriangles;
		
	public:
		TG_DWORDPool (void)
		{
			triPool = nextTri = NULL;
			totalTriangles = numTriangles = 0;
		}
		
		~TG_DWORDPool (void)
		{
			destroy();
		}
		
		void destroy (void)
		{
			TG_Shape::tglHeap->Free(triPool);
			triPool = nextTri = NULL;
			totalTriangles = numTriangles = 0;
		}
		
		void init (DWORD maxTriangles)
		{
			triPool = (DWORD *)TG_Shape::tglHeap->Malloc(sizeof(DWORD) * maxTriangles);
			gosASSERT(triPool != NULL);
			
			nextTri = triPool;
			
			totalTriangles = maxTriangles;
			numTriangles = 0;
		}
		
		void reset (void)
		{
			nextTri = triPool;
			numTriangles = 0;
		}
		
		DWORD * getFacesFromPool (DWORD numRequested)
		{
			DWORD* result = NULL;
			numTriangles += numRequested;
			if (numTriangles < totalTriangles)
			{
				result = nextTri;
				nextTri += numRequested;
			}
			
			return result;
		}
};

//-------------------------------------------------------------------------------
extern TG_VertexPool 		*colorPool;
extern TG_GOSVertexPool 	*vertexPool;
extern TG_DWORDPool			*facePool;
extern TG_ShadowPool		*shadowPool;
extern TG_TrianglePool		*trianglePool;

//-------------------------------------------------------------------------------
// ASE File Parse String Macros
#define ASE_HEADER				"*3DSMAX_ASCIIEXPORT\t200"
#define ASE_OBJECT				"*GEOMOBJECT {"
#define ASE_HELP_OBJECT			"*HELPEROBJECT {"
#define ASE_MESH				"*MESH {"
#define ASE_NUM_VERTEX			"*MESH_NUMVERTEX"
#define ASE_NUM_FACE			"*MESH_NUMFACES"
#define ASE_VERTEX_LIST			"*MESH_VERTEX_LIST {"
#define ASE_FACE_LIST			"*MESH_FACE_LIST {"
#define ASE_NUM_TVERTEX			"*MESH_NUMTVERTEX"
#define ASE_TVERTLIST			"*MESH_TVERTLIST {"
#define ASE_NUM_TVFACES			"*MESH_NUMTVFACES"
#define ASE_TFACELIST			"*MESH_TFACELIST {"
#define ASE_NUM_CVERTEX			"*MESH_NUMCVERTEX"
#define ASE_CVERTLIST			"*MESH_CVERTLIST {"
#define ASE_NUM_CVFACES			"*MESH_NUMCVFACES"
#define	ASE_CFACELIST			"*MESH_CFACELIST {"
#define ASE_MESH_NORMALS		"*MESH_NORMALS {"

#define ASE_NODE_NAME			"*NODE_NAME"
#define ASE_NODE_PARENT			"*NODE_PARENT"
#define ASE_NODE_POS			"*TM_POS"

#define ASE_ANIMATION			"*TM_ANIMATION {"
#define ASE_ANIM_ROT_HEADER		"*CONTROL_ROT_TRACK {"
#define ASE_ANIM_ROT_SAMPLE		"*CONTROL_ROT_SAMPLE"
#define ASE_ANIM_POS_HEADER		"*CONTROL_POS_TRACK {"
#define ASE_ANIM_POS_SAMPLE		"*CONTROL_POS_SAMPLE"
#define ASE_ANIM_FIRST_FRAME	"*SCENE_FIRSTFRAME"
#define ASE_ANIM_LAST_FRAME		"*SCENE_LASTFRAME"
#define ASE_ANIM_FRAME_SPEED	"*SCENE_FRAMESPEED"
#define ASE_ANIM_TICKS_FRAME	"*SCENE_TICKSPERFRAME"

#define ASE_FACE_NORMAL_ID		"*MESH_FACENORMAL"
#define ASE_VERTEX_NORMAL_ID	"*MESH_VERTEXNORMAL"
#define ASE_MESH_CFACE_ID		"*MESH_CFACE"
#define ASE_MESH_VERTCOL_ID		"*MESH_VERTCOL"
#define ASE_MESH_TFACE_ID		"*MESH_TFACE"
#define ASE_MESH_TVERT_ID		"*MESH_TVERT"
#define ASE_MESH_FACE_ID		"*MESH_FACE"
#define ASE_MESH_VERTEX_ID		"*MESH_VERTEX"

#define ASE_MATERIAL_COUNT		"*MATERIAL_COUNT"
#define ASE_SUBMATERIAL_COUNT	"*NUMSUBMTLS"
#define ASE_MATERIAL_ID			"*MATERIAL"
#define ASE_MATERIAL_BITMAP_ID	"*BITMAP "
#define ASE_MATERIAL_CLASS		"*MATERIAL_CLASS"
#define ASE_MATERIAL_TWOSIDED	"*MATERIAL_TWOSIDED"
#define ASE_FACE_MATERIAL_ID	"*MESH_MTLID"

//-------------------------------------------------------------------------------
#endif
