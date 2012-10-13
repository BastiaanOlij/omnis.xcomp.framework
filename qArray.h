/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  qArray.h
 *  template class for creating dynamic arrays of any type
 *
 *  Bastiaan Olij
 *
 *  Todos:
 *  - test code :)
 *  - add option to remove items from the array at specific locations
 *  - speed up addElements by allocating enough memory and using memcpy
 */

#ifndef qarrayh
#define qarrayh

#define QARRAY_INITIAL_SIZE		10
#define QARRAY_INCREASE_SIZE	10

template<class qElement> class qArray {
private:
	qElement		*mArray;
	unsigned long	mNumberOfElements;
	unsigned long	mMaxElements;
	
public:
	qArray();								// constructor
	~qArray();								// destructor
	
	// general access
	void *			getArray();				// return a pointer to our array
	unsigned long	numberOfElements(void);	// get the number of elements in our array
	void			addElements(qElement * pItems, unsigned long pNumberOfItems); // add an (static) array of elements
	
	// push/pop to use array as a stack
	void			clear(void);			// remove all elements from our array
	void			push(qElement * pItem);	// push an item on the stack
	qElement *		pop(void);				// pop an item from the stack
	
	// operators
	qElement *		operator [] (unsigned long pIndex);	// get an item at this index
	qArray<qElement> * operator += (qElement *pItem);		// add an item
	
	enum exceptions {
		CantAllocateMemory,
		BufferNotInitialized,
		IndexOutOfBounds
	};
};

/* implementation of template classes happens within our header file */
template<class qElement>
qArray<qElement>::qArray() {
	mNumberOfElements = 0;
	mMaxElements = QARRAY_INITIAL_SIZE;
	mArray = (qElement *) malloc(mMaxElements * sizeof(qElement));
	if (mArray == NULL) {
		throw CantAllocateMemory;
	}
}

template<class qElement>
qArray<qElement>::~qArray() {
	if (mArray != NULL) {
		free(mArray);
		mArray = NULL;
	}
}

// Returns the pointer to our array
template<class qElement>
void * qArray<qElement>::getArray() {
	return mArray;
}

// Returns the number of elements in our array
template<class qElement>
unsigned long qArray<qElement>::numberOfElements(void) {
	return mNumberOfElements;
}

// add an (static) array of elements
// This can be used with a normal static array:
// qElement Items[] = (....);
// myArray.addElements(Items, sizeof(Items) / sizeof(qElement));
// Or by adding a dynamic array:
// myArray.addElemnts(otherArray.getArray(), otherArray.numberOfElements());
template<class qElement>
void qArray<qElement>::addElements(qElement * pItems, unsigned long pNumberOfItems) {
	if (mArray == NULL) {
		throw BufferNotInitialized;
	}
	
	for (unsigned long i = 0; i<pNumberOfItems; i++) {
		push(&pItems[i]);
	}
}

// remove all elements from our array
template<class qElement>
void qArray<qElement>::clear(void) {
	// maybe in the future implement freeing up memory if array has grown in size??
	
	mNumberOfElements = 0;
}

// push an item on the stack, we get a pointer but we copy the contents of whatever the pointer points too.
template<class qElement>
void qArray<qElement>::push(qElement * pItem) {
	if (mArray == NULL) {
		throw BufferNotInitialized;
	}
	
	if (mNumberOfElements==mMaxElements) {
		mMaxElements += QARRAY_INCREASE_SIZE; // increase our size
		mArray = (qElement *)realloc(mArray, mMaxElements * sizeof(qElement));
		if (mArray == NULL) {
			throw CantAllocateMemory;
		}
	}
	
	mNumberOfElements++;
	mArray[mNumberOfElements-1] = *pItem;
}

// pop an item from the stack
template<class qElement>
qElement * qArray<qElement>::pop(void) {
	if (mArray == NULL) {
		throw BufferNotInitialized;
	}

	if (mNumberOfElements == 0) {
		throw IndexOutOfBounds;
	}
	
	mNumberOfElements--; // simply decrease our size
	return &mArray[mNumberOfElements]; // Return pointer to our last entry 
}

// get an item at this index
template<class qElement>
qElement * qArray<qElement>::operator [] (unsigned long pIndex) {
	if (mArray == NULL) {
		throw BufferNotInitialized;
	}
	
	if (pIndex>=mNumberOfElements) {
		throw IndexOutOfBounds;
	}
	
	return &mArray[pIndex];
}	

// add an item
template<class qElement>
qArray<qElement> * qArray<qElement>::operator += (qElement * pItem) {
	push(pItem);
	
	return this;
}

#endif