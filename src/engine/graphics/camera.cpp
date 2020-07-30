/* OpenHoW
 * Copyright (C) 2017-2020 TalonBrave.info and Others (see CONTRIBUTORS)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "engine.h"
#include "camera.h"

ohw::Camera::Camera( const PLVector3 &pos, const PLVector3 &angles ) {
	internalCameraPtr = plCreateCamera();
	if ( internalCameraPtr == nullptr ) {
		Error( "Failed to create camera object!\n%s\n", plGetError() );
	}
}

ohw::Camera::~Camera() {
	plDestroyCamera( internalCameraPtr );
}

/**
 * Sets the viewport position and size.
 * @param xy X and Y.
 * @param wh Width and height.
 */
void ohw::Camera::SetViewport( int x, int y, int width, int height ) {
	internalCameraPtr->viewport.x = x;
	internalCameraPtr->viewport.y = y;
	internalCameraPtr->viewport.w = width;
	internalCameraPtr->viewport.h = height;
}

void ohw::Camera::MakeActive() {
	plSetupCamera( internalCameraPtr );
}

bool ohw::Camera::IsBoxVisible( const PLCollisionAABB *bounds ) const {
	return plIsBoxInsideView( internalCameraPtr, bounds );
}

bool ohw::Camera::IsSphereVisible( const PLCollisionSphere *sphere ) const {
	return plIsSphereInsideView( internalCameraPtr, sphere );
}

void ohw::Camera::DrawViewFrustum() {
	const PLViewFrustum &frustum = internalCameraPtr->frustum;

	for ( unsigned int i = 0; i < PL_MAX_FRUSTUM_PLANES; ++i ) {
		PLVector3 startPos( 0, 0, 2.0f );
		PLVector3 endPos = PLVector3( frustum[ i ].x, frustum[ i ].y, frustum[ i ].z ) * frustum[ i ].w;
		PLMatrix4 transform;
		PLColour colour( 255, 0, 0 );
		transform = plTranslateMatrix4( internalCameraPtr->position );
		plDrawSimpleLine( &transform, &startPos, &endPos, &colour );
		transform = plMatrix4Identity();
		colour = PLColour( 0, 255, 255 );
		plDrawSimpleLine( &transform, &internalCameraPtr->position, &endPos, &colour );
	}
}
