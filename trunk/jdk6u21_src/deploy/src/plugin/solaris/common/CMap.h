/*
 * @(#)CMap.h	1.5 03/01/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CMap.h  by Stanley Man-Kit Ho
//
///=--------------------------------------------------------------------------=
//
// Contains declaration of the CMap
//

#ifndef CMAP_H_
#define CMAP_H_


/**
 * CMap is a class that provides a container functionalities. It acts as
 * a hashtable. However, currently, it is implemented using double-linked
 * list.
 * 
 * This class is a replacement for STL MAP class that was used in Java
 * Plug-in, because C++ runtime is removed in order to reduce compiled 
 * binary size.
 */
template <class X, class Y> 
class CMap
{
public:

    /** 
     * CLinkedNode represents a node to be used in a
     * double-linked list.
     */
    class CLinkedNode
    {
    public:
	// Data members
	X   first;	    // First field
	Y   second;	    // Second field
	CLinkedNode* prev;  // Pointer to previous node
	CLinkedNode* next;  // Pointer to next node

	// Constructor
	CLinkedNode(X x, Y y, 
		    CLinkedNode* pPrev = NULL, 
		    CLinkedNode* pNext = NULL)
	{
	    first = x;
	    second = y;
	    prev = pPrev;
	    next = pNext;
	}
    };

    /** 
     * iterator is an object for enumerating all the
     * elements in the container. In this case, 
     * a double-linked list.
     */
    class iterator
    {
    private:
	const CLinkedNode* m_pHead;	// Pointer to the list
	CLinkedNode*	   m_pCurrent;  // Current location of iteration3

    public:
	// Constructor
	iterator(const CLinkedNode* pHead, CLinkedNode* pCurrent)
	{
	    m_pHead = pHead;
	    m_pCurrent = pCurrent;
	}

	// Copy Constructor
	iterator(const iterator& iter) 
	{
	    m_pHead = iter.m_pHead;
	    m_pCurrent = iter.m_pCurrent;
	}


	/**
	 * operator==() is used for comparing 
	 * two iterators to check if they are
	 * equal.
	 *
	 * @return true if successful.
	 */
	bool operator==(const iterator& iter) const
	{
	    return (iter.m_pHead == m_pHead
		    && iter.m_pCurrent == m_pCurrent);
	}


	/**
	 * operator!=() is used for comparing 
	 * two iterators to check if they are
	 * not equal.
	 *
	 * @return true if successful.
	 */
	bool operator!=(const iterator& iter) const
	{
	    return !(operator==(iter));
	}


	/**
	 * operator++() is a prefix operator
	 * for moving the iterator to point
	 * to the next element.
	 *
	 * @return the iterator.
	 */
	iterator& operator++()
	{
	    if (m_pCurrent != NULL)
		m_pCurrent = m_pCurrent->next;

	    return *this;
	}


	/**
	 * operator=() is an assignment operation
	 * for copying the iterator.
	 *
	 * @return the iterator.
	 */
	iterator& operator=(const iterator& iter)
	{
	    if (this != &iter)
	    {
		m_pHead = iter.m_pHead;
		m_pCurrent = iter.m_pCurrent;
	    }

	    return *this;
	}


	/**
	 * operator++(int) is a postfix operation 
	 * for moving the iterator to 
	 * point to the next element in the list.
	 *
	 * @return the iterator.
	 */
	iterator operator++(int)
	{
	    iterator temp(m_pHead, m_pCurrent);

	    if (m_pCurrent != NULL)
		m_pCurrent = m_pCurrent->next;

	    return temp;
	}

	/**
	 * operator*() returns the current element 
	 * pointed by the iterator.
	 *
	 * @return the node.
	 */
	CLinkedNode& operator*() const
	{
	    return *m_pCurrent;
	}
    };


    // Constructor
    CMap() 
    {
	// Create a dummy node to make insert/delete easier
	m_pHead = new CLinkedNode(NULL, NULL, NULL, NULL);
    }

    // Destructor
    ~CMap()
    {
	// Clean up the whole list.
	while (m_pHead != NULL)
	{
	    CLinkedNode* pNode = m_pHead;
	    m_pHead = m_pHead->next;

	    delete pNode;
	}
    }

    /**
     * Find an element in the list.
     *
     * @param X
     * @return Y
     */
    Y FindElement(X x)
    {
	CLinkedNode* pNode = FindNode(x);

	if (pNode != NULL)
	    return pNode->second;
	else
	    return NULL;
    }


    /**
     * Insert a node into the list.
     *
     * @param X
     * @param Y
     * @return the inserted node.
     */
    void InsertElement(X x, Y y)
    {
	InsertNode(x, y);
    }

   
    /**
     * Remove a node from the list.
     *
     * @param X
     */
    void RemoveElement(X x)
    {
	RemoveNode(x);
    }


    /**
     * Returns an iterator that represents the beginning of 
     * the list.
     *
     * @return iterator
     */
    iterator begin() const
    {
	return iterator(m_pHead->next, m_pHead->next);
    }

    /**
     * Returns an iterator that represents the end of 
     * the list.
     *
     * @return iterator
     */
    iterator end() const 
    {
	return iterator(m_pHead->next, NULL);
    }

private:
    /**
     * Find a node in the list.
     *
     * @param X
     * @return the node.
     */
    CLinkedNode* FindNode(X x)
    {
	// Skip dummy node
	CLinkedNode* pCurrent = m_pHead->next;
	CLinkedNode* pPrevious = NULL;

	// Iterate the list
	while (pCurrent != NULL)
	{
	    if (pCurrent->first == x)
		return pCurrent;

	    // Goto next node
	    pCurrent = pCurrent->next;
	}

	// Cannot find the node
	return NULL;
    }

    /**
     * Insert a node into the list.
     *
     * @param X
     * @param Y
     * @return the inserted node.
     */
    CLinkedNode* InsertNode(X x, Y y)
    {
	CLinkedNode* pCurrent = FindNode(x);

	if (pCurrent != NULL)
	{
	    // Node already exist
	    pCurrent->first = x;
	    pCurrent->second = y;
	}
	else
	{
	    // No found. Insert at the head of
	    // the list
	    pCurrent = new CLinkedNode(x, y, NULL, NULL);
	    
	    pCurrent->next = m_pHead->next;
	    pCurrent->prev = m_pHead;
	    
	    if (m_pHead->next)
		m_pHead->next->prev = pCurrent;
	    
	    m_pHead->next = pCurrent;
	}

	return pCurrent;
    }

    /**
     * Remove a node from the list.
     *
     * @param X
     */
    void RemoveNode(X x)
    {
	CLinkedNode* pCurrent = FindNode(x);

	if (pCurrent)
	{
	    // Reset next pointer
	    if (pCurrent->prev != NULL)
		pCurrent->prev->next = pCurrent->next;
	
	    // Reset prev pointer
	    if (pCurrent->next != NULL)
		pCurrent->next->prev = pCurrent->prev;

	    delete pCurrent;
	}
    }

    // Points to the head of the list.
    CLinkedNode* m_pHead;
};



#endif /* CMAP_H_ */
