#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/params/Params.h"
#include "cinder/Utilities.h"
#include "cinder/ImageIo.h"
#include "CinderFreenect.h"
#include "Resources.h"

static const int VBO_X_RES  = 640;
static const int VBO_Y_RES  = 480;

using namespace ci;
using namespace ci::app;
using namespace std;

class PointCloudGl : public App {
  public:
	static void prepareSettings( Settings* settings );
	void setup();
	void createBatch();
	void update();
	void draw();
	
	// PARAMS
	params::InterfaceGlRef	mParams;
	
	// CAMERA
	CameraPersp		mCam;
	quat			mSceneRotation;
	vec3			mEye, mCenter, mUp;
	float			mCameraDistance;
	float			mKinectTilt;
	
	// KINECT AND TEXTURES
	KinectRef		mKinect;
	gl::TextureRef	mDepthTexture;
	float			mScale;
	float			mXOff, mYOff;
	
	// BATCH AND SHADER
	gl::GlslProgRef	mShader;
	gl::BatchRef	mBatch;
};

void PointCloudGl::prepareSettings( Settings* settings )
{
	settings->setWindowSize( 1280, 720 );
}

void PointCloudGl::setup()
{	
	// SETUP PARAMS
	mParams = params::InterfaceGl::create( "KinectPointCloud", ivec2( 200, 180 ) );
	mParams->addParam( "Scene Rotation", &mSceneRotation, "opened=1" );
	mParams->addParam( "Cam Distance", &mCameraDistance, "min=100.0 max=5000.0 step=100.0 keyIncr=s keyDecr=w" );
	mParams->addParam( "Kinect Tilt", &mKinectTilt, "min=-31 max=31 keyIncr=T keyDecr=t" );
	
	// SETUP CAMERA
	mCameraDistance = 1000.0f;
	mEye			= vec3( 0.0f, 0.0f, mCameraDistance );
	mCenter			= vec3( 0.0f );
	mUp				= vec3( 0.0f, 1.0f, 0.0f );
	mCam.setPerspective( 75.0f, getWindowAspectRatio(), 1.0f, 8000.0f );

	// SETUP KINECT AND TEXTURES
	mKinectTilt = 0;
	mKinect			= Kinect::create(); // use the default Kinect
	mDepthTexture	= gl::Texture::create( 640, 480 );
	
	// SETUP VBO AND SHADER	
	mShader	= gl::GlslProg::create( loadAsset( "MainVert.glsl" ), loadAsset( "MainFrag.glsl" ) );
	createBatch();
	
	// SETUP GL
	gl::enableDepthWrite();
	gl::enableDepthRead();
}

void PointCloudGl::createBatch()
{
	std::vector<vec3> positions;
	std::vector<vec2> texCoords;

	int numVertices = VBO_X_RES * VBO_Y_RES;

	auto vboMesh = gl::VboMesh::create( numVertices, GL_POINTS, { gl::VboMesh::Layout().usage( GL_STATIC_DRAW ).attrib( geom::POSITION, 3 ).attrib( geom::TEX_COORD_0, 2 ) } );

	for( int x=0; x<VBO_X_RES; ++x ){
		for( int y=0; y<VBO_Y_RES; ++y ){
			float xPer	= x / (float)(VBO_X_RES-1);
			float yPer	= y / (float)(VBO_Y_RES-1);

			positions.push_back( vec3( ( xPer * 2.0f - 1.0f ) * VBO_X_RES, ( yPer * 2.0f - 1.0f ) * VBO_Y_RES, 0.0f ) );
			texCoords.push_back( vec2( xPer, yPer ) );
		}
	}

	vboMesh->bufferAttrib( geom::POSITION, positions );
	vboMesh->bufferAttrib( geom::TEX_COORD_0, texCoords );

	mBatch = gl::Batch::create( vboMesh, mShader );
}

void PointCloudGl::update()
{
	if( mKinect->checkNewDepthFrame() )
		mDepthTexture = gl::Texture::create( mKinect->getDepthImage() );
	
	if( mKinectTilt != mKinect->getTilt() )
		mKinect->setTilt( mKinectTilt );
		
	mEye = vec3( 0.0f, 0.0f, mCameraDistance );
	mCam.lookAt( mEye, mCenter, mUp );
	gl::setMatrices( mCam );
}

void PointCloudGl::draw()
{
	gl::clear( Color( 0.0f, 0.0f, 0.0f ), true );

	gl::pushMatrices();
		gl::scale( vec3( -1.0f, 1.0f, 1.0f ) );
		gl::rotate( mSceneRotation );
		gl::ScopedTextureBind tex( mDepthTexture );
		gl::ScopedGlslProg glsl( mShader );
		mShader->uniform( "depthTex", 0 );
		mBatch->draw();
	gl::popMatrices();

	mParams->draw();
}


CINDER_APP( PointCloudGl, RendererGl, PointCloudGl::prepareSettings )
