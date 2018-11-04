#include "Galaxy.h"

Galaxy::Galaxy( GLfloat radius, glm::vec3 center )
	:radius(radius), center(center)
{
}

#ifdef DRAW_QUADTREE
std::vector<glm::vec3> Galaxy::getQuadTreeVerts() const
{
	return _root->getQuadTreeVerts(_quadTreeVerts);
}
#endif
