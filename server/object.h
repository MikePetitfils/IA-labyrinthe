enum {
ObjectCategoryNone 		= 0,
ObjectCategoryHealth 	= 1 << 0,
ObjectCategoryWeapon 	= 1 << 1,
ObjectCategoryTrap		= 1 << 2
};

typedef unsigned int ObjectCategory;

typedef struct{
	int id;	/* the object id */
	ObjectCategory category;	/* object category : health, arm, trap.. */
	char name[120]; /* the name */
	int action;	/* a function pointeur */
	int x;		/* x position of the object */
	int y;		/* y position of the object */
}Object;
