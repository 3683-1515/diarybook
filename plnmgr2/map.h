typedef struct Map {
	int key;
	void* value;
	struct Map *next;
} map;

map* createMap() 
{
	return (map*)malloc(sizeof(map));
}

destroymap(mp)
	map *mp;
{
	free(mp);
}

void* getItem(mp, key)
	map *mp;
	int key;
{
	map *pm = mp;
	while(pm) {
		if(pm->key == key) return pm->value;
		pm = pm->next;
	}
}

delItem(mp, key)
	map *mp;
	int key;
{
	map *pm, *pv = 0;
	for(pm=mp; pm; pv=pm, pm=pm->next) {
		if(pm->key == key) {
			if(pm->next) {
				
			} else if(pv) {
				
			} else {
				
			}
		}
	}
}

setItem(mp, key, value)
	map *mp;
	int key;
	void* value;
{
	
}
	


