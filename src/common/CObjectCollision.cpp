/*
 * CObjectCollision.cpp
 *
 *  Created on: 01.11.2010
 *      Author: gerstrong
 *
 *  This is the second cpp file for the CObject Class
 *  It is only meant to be used for colisions
 */

#include "CObject.h"
#include "engine/spritedefines.h"

/*
 * \brief performs collision without bouncing box recalculation
 */
void CObject::performCollisionsSameBox()
{
	// Left/Right borders
	blockedl = checkSolidL(m_Pos.x+m_BBox.x1, m_Pos.x+m_BBox.x2, m_Pos.y+m_BBox.y1, m_Pos.y+m_BBox.y2);
	blockedr = checkSolidR(m_Pos.x+m_BBox.x1, m_Pos.x+m_BBox.x2, m_Pos.y+m_BBox.y1, m_Pos.y+m_BBox.y2);

	// Upper/Lower borders
	blockedu = checkSolidU(m_Pos.x+m_BBox.x1, m_Pos.x+m_BBox.x2, m_Pos.y+m_BBox.y1);
	blockedd = checkSolidD(m_Pos.x+m_BBox.x1, m_Pos.x+m_BBox.x2, m_Pos.y+m_BBox.y2);

	if(g_pBehaviorEngine->getEpisode() > 3)
	{ // now check for the sloped tiles
		performCollisionOnSlopedTiles();
	}
}

/*
 * \brief Calculate Bouncing Boxes with extra placement.
 */
void CObject::calcBouncingBoxeswithPlacement()
{
	CSprite &rSprite = g_pGfxEngine->getSprite(sprite);

    const int diff_y =  m_BBox.y2==0 ? 0 :(int)m_BBox.y2-(int)rSprite.m_bboxY2;

    calcBouncingBoxes();

    moveYDir(diff_y);
}

/*
 * \brief Calculate Bouncing Boxes
 */
void CObject::calcBouncingBoxes()
{
	CSprite &rSprite = g_pGfxEngine->getSprite(sprite);

	m_BBox.x1 = rSprite.m_bboxX1;
	m_BBox.x2 = rSprite.m_bboxX2;
   	m_BBox.y1 = rSprite.m_bboxY1;
   	m_BBox.y2 = rSprite.m_bboxY2;
}


/*
 * This function determines if the object is touching a sloped tile
 * and determine the collisions
 */
const int COLISION_RES = (1<<STC);
void CObject::performCollisionOnSlopedTiles()
{
	const Uint32 halftile = ((1<<CSF)/2);
	const Uint32 x1 = m_Pos.x + m_BBox.x1;
	const Uint32 x2 = m_Pos.x + m_BBox.x2;
	const Uint32 y2 = m_Pos.y + m_BBox.y2;
	std::vector<CTileProperties> &TileProperty = g_pBehaviorEngine->getTileProperties();
	onslope = false;

	for(Uint32 c=y2-halftile ; c<=y2+halftile ; c += COLISION_RES)
	{
		char block;
		block = TileProperty[mp_Map->at(x1>>CSF, c>>CSF)].bup;
		if( block >= 2 && block <= 7 )
		{
			onslope = true;
			break;
		}
		block = TileProperty[mp_Map->at(x2>>CSF, c>>CSF)].bup;
		if( block >= 2 && block <= 7 )
		{
			onslope = true;
			break;
		}
	}

	if(!onslope)
	{
		for(Uint32 c=x1 ; c<=x2 ; c += COLISION_RES)
		{
			char block;
			block = TileProperty[mp_Map->at(c>>CSF, (y2+halftile)>>CSF)].bup;
			if( block >= 2 && block <= 7 )
			{
				onslope = true;
				break;
			}
		}
	}

	if(onslope)
	{
		blockedr = blockedl = false;
	}
}
/**
 * So far only used in Galaxy. This is the code for sloped tiles downside
 * This is performed when Keen walks into a sloped tile
 *
 * 0	Fall through		1	Flat
 * 2	Top -> Middle		3	Middle -> bottom
 * 4	Top -> bottom		5	Middle -> top
 * 6	Bottom -> middle	7	Bottom -> top
 * 8	Unused			9	Deadly, can't land on in God mode
 */
void getSlopePointsLowerTile(char slope, int &yb1, int &yb2)
{
	// Calculate the corner y coordinates
	if( slope == 2 )
		yb1 = 0,	yb2 = 256;
	else if( slope == 3 )
		yb1 = 256,	yb2 = 512;
	else if( slope == 4 )
		yb1 = 0,	yb2 = 512;
	else if( slope == 5 )
		yb1 = 256,	yb2 = 0;
	else if( slope == 6 )
		yb1 = 512,	yb2 = 256;
	else if( slope == 7 )
		yb1 = 512,	yb2 = 0;
	else
		yb1 = 0, yb2 = 0;
}

/*
 * \brief This checks the collision. Very simple pixel based algorithm
 * 		  The collision is per pixel-based
 */
void CObject::performCollisions()
{
	blockedr = blockedl = false;
	blockedu = blockedd = false;

	if ( sprite != BLANKSPRITE )
	{
		calcBouncingBoxes();
		performCollisionsSameBox();
	}
}

// Basic slope move independent of the left or right move
void CObject::moveSlopedTiles( int x, int y1, int y2, int xspeed )
{
	// process the sloped tiles here. Galaxy only or special patch!!
	if(g_pBehaviorEngine->getEpisode() > 3)
	{
		if(!moveSlopedTileDown(x, y2, xspeed))
			moveSlopedTileUp(x, y1, xspeed);
	}
}

bool CObject::moveSlopedTileDown( int x, int y, int xspeed )
{
	if(yinertia<0)
		return false;

	std::vector<CTileProperties> &TileProperty = g_pBehaviorEngine->getTileProperties();

	const char slope = TileProperty[mp_Map->at(x>>CSF, y>>CSF)].bup;

	// Check first, if there is a tile on players level
	if( slope >=2 && slope<=7 )
	{
		int yb1, yb2;
		getSlopePointsLowerTile(slope, yb1, yb2);

		// Get relative position of the X corner
		const int L = 512;
		const int x_r = (x%L);

		// get the dy position so the new pos can be computed
		int dy = ((yb2-yb1)*x_r)/L;

		// get the new position of the lower corner
		const int y_rel = yb1 + dy;

		// Get the upper border pos if the tile
		int y_csf;
		y_csf = (y>>CSF)<<CSF;

		// Get the new lower corner pos
		const int y_pos = y_csf + y_rel;

		// get new position
		Uint32 new_y;

		// Now we have to see all the cases for sloped tiles of an object
		if(xspeed > 0) // when going right...
		{
			new_y = y_pos - m_BBox.y2;
			if( x_r >= 480 ) // At Tile edge
			{
				if( yb1>yb2 )
				{
					new_y = (new_y>>CSF)<<CSF;
					dy = m_Pos.y - (new_y+yb2);
					moveUp( dy );
					moveRight( dy );
				}
				else if( yb1<yb2 )
				{
					new_y = (new_y>>CSF)<<CSF;
					dy = m_Pos.y - (new_y+yb2);
					moveDown( -dy );
				}
			}
			else // In the Tile itself or walking into...
			{
				moveYDir( new_y - m_Pos.y );
			}
		}
		else if(xspeed < 0) // Going left
		{
			new_y = y_pos - m_BBox.y2;
			if( x_r <= 32 ) // At Tile edge
			{
				if( yb1<yb2 )
				{
					new_y = (new_y>>CSF)<<CSF;
					dy = (new_y+yb1) - m_Pos.y;
					moveYDir( dy );
					moveLeft( dy );
				}
				else if( yb1>yb2 )
				{
					new_y = (new_y>>CSF)<<CSF;
					dy = (new_y+yb1) - m_Pos.y;
					moveDown( -dy );
				}
			}
			else // In the Tile itself or walking into...
			{
				moveYDir( new_y - m_Pos.y );
			}
		}
		return true;
	}
	else
		return false;
}

/**
 * So far only used in Galaxy. This is the code for sloped tiles upside
 *
 * 0	Jump through		1	Flat bottom
 * 2	Bottom-> Middle		3	Middle -> top
 * 4	Bottom -> top		5	Middle -> bottom
 * 6	Top -> middle		7	Top -> bottom
 *
 */
void CObject::moveSlopedTileUp( int x, int y, int xspeed )
{
	std::vector<CTileProperties> &TileProperty = g_pBehaviorEngine->getTileProperties();
	char slope = TileProperty[mp_Map->at(x>>CSF, y>>CSF)].bdown;

	// Let's calculate the exact Y-Coordinate and put the object there...

	// Calculate the corner y coordinates
	int yb1, yb2;

	if( slope == 2 )
		yb1 = 512,	yb2 = 256;
	else if( slope == 3 )
		yb1 = 256,	yb2 = 0;
	else if( slope == 4 )
		yb1 = 512,	yb2 = 0;
	else if( slope == 5 )
		yb1 = 256,	yb2 = 512;
	else if( slope == 6 )
		yb1 = 0,	yb2 = 256;
	else if( slope == 7 )
		yb1 = 0,	yb2 = 512;
	else
		return;

	// Get the upper border pos if the tile
	const int y_csf = (y>>CSF)<<CSF;

	// Get relative position of the X corner
	const int L = 512;
	const int x_r = (x%L);

	// get the dy position so the new pos can be computed
	int dy = ((yb2-yb1)*x_r)/L;

	// get the new position of the lower corner
	const int y_rel = yb1 + dy;

	// Get the new lower corner pos
	const int y_pos = y_csf + y_rel;

	// get new position
	const Uint32 new_y = y_pos - m_BBox.y1 + (1<<STC);
	moveYDir( new_y - m_Pos.y );
}

// returns nonzero if object1 overlaps object2
bool CObject::hitdetect(CObject &hitobject)
{
	unsigned int rect1x1, rect1y1, rect1x2, rect1y2;
	unsigned int rect2x1, rect2y1, rect2x2, rect2y2;

	// get the bounding rectangle of the first object
	rect1x1 = m_Pos.x + m_BBox.x1;
	rect1y1 = m_Pos.y + m_BBox.y1;
	rect1x2 = m_Pos.x + m_BBox.x2;
	rect1y2 = m_Pos.y + m_BBox.y2;

	// get the bounding rectangle of the second object
	rect2x1 = hitobject.getXPosition() + hitobject.m_BBox.x1;
	rect2y1 = hitobject.getYPosition() + hitobject.m_BBox.y1;
	rect2x2 = hitobject.getXPosition() + hitobject.m_BBox.x2;
	rect2y2 = hitobject.getYPosition() + hitobject.m_BBox.y2;

	// find out if the rectangles overlap
	if ((rect1x1 <= rect2x1) && (rect1x2 <= rect2x1)) return false;
	if ((rect1x1 >= rect2x2) && (rect1x2 >= rect2x2)) return false;
	if ((rect1y1 <= rect2y1) && (rect1y2 <= rect2y1)) return false;
	if ((rect1y1 >= rect2y2) && (rect1y2 >= rect2y2)) return false;

	return true;
}


/**
 * \brief this new type of hit detection only checks if the foe touches something that has that property tile
 * \param Property The Tile Property we are looking
 * \param from x
 * \return true if detection worked with that tile having the property, else false
 */
bool CObject::hitdetectWithTilePropertyRect(const Uint16 Property, int &lx, int &ly, int &lw, int &lh, const int res)
{
	std::vector<CTileProperties> &Tile = g_pBehaviorEngine->getTileProperties(1);

	for( int i=0 ; i<lw ; i+=res )
	{
		for( int j=0 ; j<lh ; j+=res )
		{
			const char behavior = Tile[mp_Map->getPlaneDataAt(1, lx+i, ly+j)].behaviour;
			if(behavior == Property || behavior == Property-128 ) // -128 for foreground properties
			{
				lx = lx+i;
				ly = ly+j;
				return true;
			}
		}
	}
	return false;
}


/**
 * \brief this new type of hit detection only checks if the foe touches something that has that property
 * \param Property The Tile Property we are looking
 * \return true if detection worked with that tile having the property, else false
 */
bool CObject::hitdetectWithTileProperty(const int& Property, const int& x, const int& y)
{
	std::vector<CTileProperties> &Tile = g_pBehaviorEngine->getTileProperties(1);
	const char behavior = Tile[mp_Map->getPlaneDataAt(1, x, y)].behaviour;
	if(behavior == Property || behavior == Property-128 ) // +128 for foreground properties
		return true;
	else
		return false;
}

int CObject::checkSolidR( int x1, int x2, int y1, int y2)
{
	std::vector<CTileProperties> &TileProperty = g_pBehaviorEngine->getTileProperties();
	int blocker;
	bool vorticon = (g_pBehaviorEngine->getEpisode() <= 3);

	x2 += COLISION_RES;

	if( (x2>>STC) != ((x2>>CSF)<<TILE_S) )
		return false;

	// Check for right from the object
	if(solid)
	{
		for(int c=y1 ; c<=y2 ; c += COLISION_RES)
		{
			blocker = TileProperty[mp_Map->at(x2>>CSF, c>>CSF)].bleft;
			if(blocker)
				return blocker;
		}

		blocker = TileProperty[mp_Map->at(x2>>CSF, y2>>CSF)].bleft;
		if(blocker)
			return blocker;
	}

	if( m_type == OBJ_PLAYER && solid )
	{
		if(vorticon)
		{
			if( x2 >= (int)((mp_Map->m_width-2)<<CSF) ) return 1;
		}
		else
		{
			if( x2 >= (int)((mp_Map->m_width-1)<<CSF) ) return 1;
		}

	}
	else
	{
		if( (Uint16)x2 > ((mp_Map->m_width)<<CSF) )
		{
			exists=false; // Out of map?
			return 1;
		}
	}

	return 0;
}

int CObject::checkSolidL( int x1, int x2, int y1, int y2)
{
	bool vorticon = (g_pBehaviorEngine->getEpisode() <= 3);
	int blocker;
	std::vector<CTileProperties> &TileProperty = g_pBehaviorEngine->getTileProperties();

	x1 -= COLISION_RES;

	// Check for right from the object
	if(solid)
	{
		for(int c=y1 ; c<=y2 ; c += COLISION_RES)
		{
			blocker = TileProperty[mp_Map->at(x1>>CSF, c>>CSF)].bright;
			if(blocker)
				return blocker;
		}

		blocker = TileProperty[mp_Map->at(x1>>CSF, y2>>CSF)].bright;
		if(blocker)
			return blocker;
	}

	// borders
	if( m_type == OBJ_PLAYER && solid )
	{
		if(vorticon)
			if( x1 <= (2<<CSF) ) return 1;
		else
			if( x1 <= (1<<CSF) ) return 1;
	}
	else
	{
		if( x1 == 0 )
		{
			exists=false; // Out of map?
			return 1;
		}
	}
	return 0;
}

int CObject::checkSolidU(int x1, int x2, int y1)
{
	bool vorticon = (g_pBehaviorEngine->getEpisode() <= 3);
	int blocker;
	std::vector<CTileProperties> &TileProperty = g_pBehaviorEngine->getTileProperties();

	y1 -= COLISION_RES;

	// Check for sloped tiles here. They must be handled differently
	if(!vorticon && solid)
	{
		char blocked;

		if(m_climbing)
		{
			x1 += 4*COLISION_RES;
			x2 -= 4*COLISION_RES;
		}

		for(int c=x1 ; c<=x2 ; c += COLISION_RES)
		{
			blocked = TileProperty[mp_Map->at(c>>CSF, y1>>CSF)].bdown;

			if(blocked == 17 && m_climbing)
				return 0;

			if( blocked >= 2 && blocked <= 7 && checkslopedU(c, y1, blocked))
				return blocked;
		}
		blocked = TileProperty[mp_Map->at((x2-(1<<STC))>>CSF, y1>>CSF)].bdown;
		if( blocked >= 2 && blocked <= 7 && checkslopedU(x2-(1<<STC), y1, blocked ))
			return true;

		if(blocked == 17 && m_climbing)
			return false;
	}

	if( ((y1+COLISION_RES)>>STC) != (((y1+COLISION_RES)>>CSF)<<TILE_S)  )
		return false;

	// Check for right from the object
	if(solid)
	{
		for(int c=x1 ; c<=x2 ; c += COLISION_RES)
		{
			char blocked = TileProperty[mp_Map->at(c>>CSF, y1>>CSF)].bdown;

			if(blocked)
			{
				if(vorticon)
					return true;
				else if(blocked == 1)
					return true;
			}
		}
	}

	if( y1 <= ( ((m_type == OBJ_PLAYER) ? 2 : 1)<<CSF) )
		return true;

	return false;
}

int CObject::checkSolidD( int x1, int x2, int y2 )
{
	bool vorticon = (g_pBehaviorEngine->getEpisode() <= 3);
	int blocker;
	std::vector<CTileProperties> &TileProperty = g_pBehaviorEngine->getTileProperties();

	y2 += COLISION_RES;

	// Check for sloped tiles here. They must be handled different
	if(!vorticon && solid)
	{
		char blocked;

		if(m_climbing)
		{
			x1 += 4*COLISION_RES;
			x2 -= 4*COLISION_RES;
		}

		for(int c=x1 ; c<=x2 ; c += COLISION_RES)
		{
			blocked = TileProperty[mp_Map->at(c>>CSF, y2>>CSF)].bup;

			if(blocked == 17 && m_climbing)
				return false;

			if( blocked >= 2 && blocked <= 7 && checkslopedD(c, y2, blocked) )
			//if( blocked )
				return true;
		}

		blocked = TileProperty[mp_Map->at((x2)>>CSF, y2>>CSF)].bup;

		if(blocked == 17 && m_climbing)
			return false;

		if( blocked >= 2 && blocked <= 7 && checkslopedD(x2, y2, blocked) )
		//if( blocked )
			return true;
	}

	if( (y2>>STC) != ((y2>>CSF)<<TILE_S) )
		return false;

	// Check for down from the object
	if(solid)
	{
		char blocked;
		for(int c=x1 ; c<=x2 ; c += COLISION_RES)
		{
			blocked = TileProperty[mp_Map->at(c>>CSF, y2>>CSF)].bup;

			if(blocked)
			{
				if( blocked < 2 || blocked > 7 )
					return true;
			}
		}

		blocked = TileProperty[mp_Map->at((x2-(1<<STC))>>CSF, y2>>CSF)].bup;
		if(blocked)
		{
			if( blocked < 2 || blocked > 7 )
				return true;
		}
	}

	if( (Uint32)y2 > ((mp_Map->m_height)<<CSF) )
		exists=false; // Out of map?

	// This is a special case for foes which can turn around when they walk over an edge before they fall
	if(m_canturnaround &&
		( !TileProperty[mp_Map->at((x1-(1<<STC))>>CSF, (y2+(1<<STC))>>CSF)].bup ||
		  !TileProperty[mp_Map->at((x2+(1<<STC))>>CSF, (y2+(1<<STC))>>CSF)].bup ) )
	{
		blockedl = TileProperty[mp_Map->at((x2+(1<<STC))>>CSF, (y2+(1<<STC))>>CSF)].bup;
		blockedr = TileProperty[mp_Map->at((x1-(1<<STC))>>CSF, (y2+(1<<STC))>>CSF)].bup;

		return true;
	}

	return false;
}

/* Special slope function for galaxy maps
	0	Jump through		1	Flat bottom
	2	Bottom-> Middle		3	Middle -> top
	4	Bottom -> top		5	Middle -> bottom
	6	Top -> middle		7	Top -> bottom
 */
bool CObject::checkslopedU( int c, int y1, char blocked)
{
	int yb1, yb2;

	if( blocked == 2 )
		yb1 = 512,	yb2 = 256;
	else if( blocked == 3 )
		yb1 = 256,	yb2 = 0;
	else if( blocked == 4 )
		yb1 = 512,	yb2 = 0;
	else if( blocked == 5 )
		yb1 = 256,	yb2 = 512;
	else if( blocked == 6 )
		yb1 = 0,	yb2 = 256;
	else if( blocked == 7 )
		yb1 = 0,	yb2 = 512;
	else
		yb1 = 512,	yb2 = 512;

	int dy = ((yb2-yb1-32)*(c%512))/512;
	int yh = yb1 + dy;

	return ( (y1%512) < yh );
}

/* Special slope function for galaxy maps
0	Fall through		1	Flat
2	Top -> Middle		3	Middle -> bottom
4	Top -> bottom		5	Middle -> top
6	Bottom -> middle	7	Bottom -> top
8	Unused				9	Deadly, can't land on in God mode
 */
bool CObject::checkslopedD( int c, int y2, char blocked)
{
	int yb1, yb2;

	getSlopePointsLowerTile(blocked, yb1, yb2);

	int dy = ((yb2-yb1)*(c%512))/512;
	int yh = yb1 + dy;

	return ( y2%512 > yh );
}





//====================================================================================
//====================================================================================
//====================================================================================
//====================================================================================
//====================================================================================
//====================================================================================
//====================================================================================
//====================================================================================
//====================================================================================

//====================================================================================

// New Colission based on events and adjustments. read below

//====================================================================================
void CObject::doBouncingBoxResizal(const BouncingBox& new_BBox)
{
	std::vector<CTileProperties> &TileProperty = g_pBehaviorEngine->getTileProperties();

	// Setup up the new bouncing box
	m_BBox = new_BBox;
	unsigned int x1 = getXPosition()+m_BBox.x1;
	unsigned int x2 = getXPosition()+m_BBox.x2;
	unsigned int y1 = getYPosition()+m_BBox.y1;
	unsigned int y2 = getYPosition()+m_BBox.y2;


	for(unsigned int c=y1 ; c<=y2 ; c += COLISION_RES)
	{
		// Adjust the left side
		while(TileProperty[mp_Map->at(x1>>CSF, c>>CSF)].bright)
		{
			move(1,0);
			x1 = getXPosition()+m_BBox.x1;
		}

		// Adjust the right side
		while(TileProperty[mp_Map->at(x2>>CSF, c>>CSF)].bleft)
		{
			move(-1,0);
			x2 = getXPosition()+m_BBox.x2;
		}
	}

	for(unsigned int c=x1 ; c<=x2 ; c += COLISION_RES)
	{
		// Adjust the lower side
		while(TileProperty[mp_Map->at(c>>CSF, y1>>CSF)].bup)
		{
			move(0,-1);
			y1 = getYPosition()+m_BBox.y1;
		}

		// Adjust the upper side
		while(TileProperty[mp_Map->at(c>>CSF, y2>>CSF)].bdown)
		{
			move(0,1);
			y2 = getYPosition()+m_BBox.y2;
		}
	}
}

void CObject::moveBitLeft()
{
	/// Now check the neighboring tile to the left
	const unsigned int x1 = getXPosition()+m_BBox.x1;
	const unsigned int x2 = getXPosition()+m_BBox.x2;
	const unsigned int y1 = getYPosition()+m_BBox.y1;
	const unsigned int y2 = getYPosition()+m_BBox.y2;

	if( (blockedl = checkSolidL(x1, x2, y1, y2)) == true)
	{
		return;
	}

	// if we are here, the tiles aren't blocking us.
	// TODO: Here we need the Object collision part
	m_Pos.x--;
}

void CObject::moveBitRight()
{
	/// Now check the neighboring tile to the right
	const unsigned int x1 = getXPosition()+m_BBox.x1;
	const unsigned int x2 = getXPosition()+m_BBox.x2;
	const unsigned int y1 = getYPosition()+m_BBox.y1;
	const unsigned int y2 = getYPosition()+m_BBox.y2;

	if( (blockedr = checkSolidR(x1, x2, y1, y2)) == true)
		return;

	// if we are here, the tiles aren't blocking us.
	// TODO: Here we need the Object collision part
	m_Pos.x++;
}

void CObject::moveBitUp()
{
	/// Now check the neighboring tile to the up
	const unsigned int x1 = getXPosition()+m_BBox.x1;
	const unsigned int x2 = getXPosition()+m_BBox.x2;
	const unsigned int y1 = getYPosition()+m_BBox.y1;

	if( (blockedu = checkSolidU(x1, x2, y1)) == true)
		return;

	// if we are here, the tiles aren't blocking us.
	// TODO: Here we need the Object collision part
	m_Pos.y--;
}

void CObject::moveBitDown()
{
	/// Now check the neighboring tile to the down
	const unsigned int x1 = getXPosition()+m_BBox.x1;
	const unsigned int x2 = getXPosition()+m_BBox.x2;
	const unsigned int y2 = getYPosition()+m_BBox.y2;

	if( (blockedd = checkSolidD(x1, x2, y2)) == true)
		return;

	// if we are here, the tiles aren't blocking us.
	// TODO: Here we need the Object collision part
	m_Pos.y++;
}

void CObject::move(const VectorD2<int>& dir)
{
	move(dir.x, dir.y);
}

void CObject::move(const int xoff, const int yoff)
{
	// Let's check if we have to move left or right
	if(xoff>0)
	{
		// move right
		for(int c=0 ; c<xoff ; c++ )
			moveBitRight();
	}
	else if(xoff<0)
	{
		// move right
		for(int c=0 ; c<-xoff ; c++ )
			moveBitLeft();
	}

	// Let's check if we have to move up or down
	if(yoff>0)
	{
		// move down
		for(int c=0 ; c<yoff ; c++ )
			moveBitDown();
	}
	else if(yoff<0)
	{
		// move right
		for(int c=0 ; c<-yoff ; c++ )
			moveBitUp();
	}
}

void CObject::processEvents()
{
	while(!m_EventCont.empty())
	{
		if( ObjResizeBB* p_ObjResizeBB =  m_EventCont.occurredEvent<ObjResizeBB>())
		{
			doBouncingBoxResizal(p_ObjResizeBB->m_BB);
			m_EventCont.pop_Event();
		}

		if( ObjMove* p_ObjMove =  m_EventCont.occurredEvent<ObjMove>())
		{
			move(p_ObjMove->m_Vec);
			m_EventCont.pop_Event();
		}
	}
}
