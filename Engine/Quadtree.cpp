#include "Quadtree.h"
#include "GameObject.h"
#include "Geometry/AABB.h"
#include "GL3W/include/glew.h"
// QUADTREE NODE -------------------------

enum QuadTreeChild
{
	FRONT_LEFT = 0,
	FRONT_RIGHT,
	BACK_RIGHT,
	BACK_LEFT
};

QuadtreeNode::QuadtreeNode(const AABB& box) : box(box)
{
	parent = nullptr;

	for (uint i = 0; i < 4; i++)
	{
		childs[i] = nullptr;
	}
}

QuadtreeNode::~QuadtreeNode()
{
	for (uint i = 0; i < 4; i++)
	{
		if (childs[i] != nullptr)
		{
			childs[i]->objects.clear();
			RELEASE(childs[i]);
		}
	}
}

bool QuadtreeNode::isLeaf() const
{
	if (childs[0] == nullptr)
	{
		return true;
	}

	return false;
}

void QuadtreeNode::Insert(GameObject* obj)
{
	if (isLeaf() && (objects.size() < QUADTREE_MAX_ITEMS ||
		(box.HalfSize().LengthSq() <= QUADTREE_MIN_SIZE * QUADTREE_MIN_SIZE)))
	{
		objects.push_back(obj);
	}

	else
	{
		if (isLeaf())
		{
			CreateChilds();
		}
		objects.push_back(obj);

		RedistributeChilds();
	}
}

void QuadtreeNode::Remove(GameObject* obj)
{
	std::list<GameObject*>::iterator it = std::find(objects.begin(), objects.end(), obj);

	if (it != objects.end())
	{ 
		objects.erase(it);
	}

	if (!isLeaf())
	{
		for (uint i = 0; i < 4; i++)
		{
			childs[i]->Remove(obj);
		}
	}
}

void QuadtreeNode::Clear()
{
	if (childs[0] != nullptr)
	{
		for (uint i = 0; i < 4; i++)
		{
			childs[i]->objects.clear();
			RELEASE(childs[i]);
		}
	}
}

void QuadtreeNode::DebugDraw()
{
	for (uint i = 0; i < 12; i++)
	{
		glVertex3f(box.Edge(i).a.x, box.Edge(i).a.y, box.Edge(i).a.z);
		glVertex3f(box.Edge(i).b.x, box.Edge(i).b.y, box.Edge(i).b.z);
	}

	if (childs[0] != nullptr)
	{
		for (uint i = 0; i < 4; i++)
		{
			childs[i]->DebugDraw();
		}
	}
}

void QuadtreeNode::CreateChilds()
{
	// We divide the node into 4 equal parts
	float3 size(box.Size());
	float3 size_new(size.x*0.5f, size.y, size.z*0.5f);

	float3 center(box.CenterPoint());
	float3 center_new;
	
	AABB box_new;

	// -X / -Z
	center_new.Set(center.x - size_new.x * 0.5f, center.y, center.z - size_new.z * 0.5f);
	box_new.SetFromCenterAndSize(center_new, size_new);
	childs[FRONT_LEFT] = new QuadtreeNode(box_new);

	// +X / -Z
	center_new.Set(center.x + size_new.x * 0.5f, center.y, center.z - size_new.z * 0.5f);
	box_new.SetFromCenterAndSize(center_new, size_new);
	childs[FRONT_RIGHT] = new QuadtreeNode(box_new);

	// +X / +Z
	center_new.Set(center.x + size_new.x * 0.5f, center.y, center.z + size_new.z * 0.5f);
	box_new.SetFromCenterAndSize(center_new, size_new);
	childs[BACK_RIGHT] = new QuadtreeNode(box_new);

	// -X / +Z
	center_new.Set(center.x - size_new.x * 0.5f, center.y, center.z + size_new.z * 0.5f);
	box_new.SetFromCenterAndSize(center_new, size_new);
	childs[BACK_LEFT] = new QuadtreeNode(box_new);
}

void QuadtreeNode::RedistributeChilds()
{
	// We distribute the Game Objects depending on its position respect to the new childs
	GameObject* object = nullptr;

	std::list<GameObject*>::iterator it;
	for (it = objects.begin(); it != objects.end();)
	{
		object = *it;

		// Check intersections between all 4 childs and the Game Object
		bool intersecting[4];
		uint num_intersections = 0;
		for (uint i = 0; i < 4; i++)
		{
			intersecting[i] = childs[i]->box.Intersects(*object->bounding_box);
			num_intersections++;
		}

		if (num_intersections == 4)
		{
			it++; // Game Object mantains its original relation with the parent
		}
		else
		{
			for (uint i = 0; i < 4; i++)
			{
				if (intersecting[i])
				{
					// Insert the Game Object into the correct child
					childs[i]->Insert(object);
				}
			}
		}
	}
}

// ---------------------------------------


// QUADTREE ------------------------------
Quadtree::Quadtree()
{
}

Quadtree::~Quadtree()
{
}

void Quadtree::Boundaries(AABB limits)
{
	Clear();
	root_node = new QuadtreeNode(limits);
}

void Quadtree::Clear()
{
	if (root_node != nullptr)
	{
		root_node->objects.clear();
		RELEASE(root_node);
	}	
}

void Quadtree::Bake(const std::vector<GameObject*>& objects)
{
	for (uint i = 0; i < objects.size(); i++)
	{
		if (!objects[i]->isStatic())
		{
			Insert(objects[i]);

			if (objects[i]->GetNumChilds() > 0)
			{
				Bake(objects[i]->GetChildsVec());
			}
		}
	}
}

void Quadtree::Insert(GameObject* obj)
{
	if (root_node != nullptr)
	{
		// If object is inside the Root Bounding box, insert it in a node
		if (obj->bounding_box != nullptr)
		{
			if (obj->bounding_box->Intersects(root_node->box))
			{
				root_node->Insert(obj);
			}
		}
		else
		{
			LOG("The Game Object you are trying to Insert hasn't got an AABB.");
		}
	}
}

void Quadtree::Remove(GameObject* obj)
{
	if (root_node != nullptr)
	{
		root_node->Remove(obj);
	}
}

void Quadtree::DebugDraw()
{
	glBegin(GL_LINES);
	glLineWidth(3.0f);
	glColor4f(1.00f, 0.761f, 0.00f, 1.00f);

	if(root_node != nullptr)
	{
		root_node->DebugDraw();
	}

	glEnd();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

// --------------------------------------
