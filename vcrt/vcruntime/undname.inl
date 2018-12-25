//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft shared
// source or premium shared source license agreement under which you licensed
// this source code. If you did not accept the terms of the license agreement,
// you are not authorized to use this source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the SOURCE.RTF on your install media or the root of your tools installation.
// THE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
/*
 *	This module contains the definitions for the inline functions used by the
 *	name undecorator.  It is intended that this file should be included
 *	somewhere in the source file for the undecorator to maximise the chance
 *	that they will be truly inlined.
 */

//	The following class is a special node class, used in the implementation
//	of the internal chaining mechanism of the 'DName's

class	charNode;
class	pcharNode;
class	pDNameNode;
class DNameStatusNode;


#if	( NO_VIRTUAL )
enum	NodeType
{
	charNode_t,
	pcharNode_t,
	pDNameNode_t,
	DNameStatusNode_t

};
#endif	// NO_VIRTUAL


class DNameNode
{
private:

#if	NO_VIRTUAL
		NodeType			typeIndex;
#endif	// NO_VIRTUAL

		mutable DNameNode *			next;

protected:

#if	( !NO_VIRTUAL )
					__near	DNameNode ();
#else	// } elif NO_VIRTUAL {
					__near	DNameNode ( NodeType );
#endif	// NO_VIRTUAL


public:

	virtual	int			__near	length () const PURE;
	virtual	char		__near	getLastChar () const PURE;
	virtual	pchar_t		__near	getString ( pchar_t, pchar_t ) const PURE;
	const DNameNode *	__near	nextNode () const;

	const DNameNode &	__near	operator +=(const DNameNode * ) const;
};


class charNode : public DNameNode {

public:
	charNode (char ch)
		:	me(ch)
#if	NO_VIRTUAL
		,	DNameNode ( charNode_t )
#endif
	{}

	int length() const override { return 1; }
	char getLastChar() const override { return me; }
	
	pchar_t getString(pchar_t buf, pchar_t end) const override {
		if (buf < end)
			*buf++ = me;
		return buf;
	}
private:
	char me;
};


class pairNode : public DNameNode {
public:
	DNameNode *first, *second;

	pairNode(DNameNode *f, DNameNode *s)
		:	first(f)
		,	second(s)
		,	m_length(-1)
	{}

	int	length () const override {
		if (m_length < 0)
			m_length = first->length() + second->length();
		return m_length;
	}

	pchar_t	getString(pchar_t buf, pchar_t end) const override {
		pchar_t r = first->getString(buf, end);
		return r>=end ? r : second->getString(r, end);
	}

	char getLastChar() const override {
		if (char r = second->getLastChar())
			return r;
		return first->getLastChar();
	}
private:
	mutable int m_length;
};

static pchar_t getStringHelper(pchar_t buf, pchar_t end, pchar_t src, size_t len) {
	if (len > size_t(end-buf))
		len = size_t(end-buf);
	und_memcpy(buf, src, (unsigned)len);
	return &buf[len];
}

class pcharNode	: public DNameNode {
public:
	pcharNode(pcchar_t, size_t = 0);

	int	length() const override {
		return	(int)myLen;
	}

	char getLastChar() const override {
		return	( myLen ? me[ myLen - 1 ] : '\0' );
	}

	pchar_t	getString(pchar_t buf, pchar_t end) const override {
		return getStringHelper(buf, end, me, myLen);
	}
private:
	pchar_t				me;
	size_t					myLen;
};

class pDNameNode : public DNameNode {
public:
	pDNameNode(DName *pName)
		:	me(( pName && (( pName->status () == DN_invalid ) || ( pName->status () == DN_error ))) ? 0 : pName)
#if	NO_VIRTUAL
		,	DNameNode (pDNameNode_t)
#endif
	{}

	int	length() const override { return me ? me->length() : 0; }
	char getLastChar() const override { return me ? me->getLastChar () : '\0'; }
	
	pchar_t	getString(pchar_t buf, pchar_t end) const override {
		return me ? me->getString(buf, end) : 0;
	}
private:
	DName *me;
};



class DNameStatusNode : public DNameNode {
#	define	TruncationMessage		(" ?? ")
#	define	TruncationMessageLength	(4)
public:
	DNameStatusNode(DNameStatus stat)
		:	me(stat)
#if	NO_VIRTUAL
		,	DNameNode(DNameStatusNode_t)
#endif
	{
		myLen = (( me == DN_truncated ) ? TruncationMessageLength : 0 );
	}

	static DNameStatusNode *make(DNameStatus st) {
		static DNameStatusNode s_validNode(DN_valid),
			s_truncatedNode(DN_truncated),
			s_invalidNode(DN_invalid),
			s_errorNode(DN_error);

		switch (st) {
		case DN_valid: return &s_validNode;
		case DN_truncated: return &s_truncatedNode;
		case DN_invalid: return &s_invalidNode;
		default:
			return &s_errorNode;
		}
	}

	int length() const override { return myLen; }

	char getLastChar() const override {
		return me==DN_truncated ? TruncationMessage[ TruncationMessageLength - 1 ] : '\0';
	}

	pchar_t getString(pchar_t buf, pchar_t end) const override {
		return me!=DN_truncated ? buf : getStringHelper(buf, end, TruncationMessage, TruncationMessageLength);
	}
private:
	DNameStatus	me;
	int	myLen;
};

//	Memory allocation functions
			
inline	void __far *	__near __pascal	operator new (size_t sz, _HeapManager &, int noBuffer )
{	return	heap.getMemory ( sz, noBuffer );	}

void __far *	__near	_HeapManager::getMemory (size_t sz, int noBuffer )
{
	//	Align the allocation on an appropriate boundary

	sz	= (( sz + PACK_SIZE-1 ) & ~(PACK_SIZE-1) );

	if	( noBuffer )
		return	( *pOpNew )( sz );
	else
	{
		//	Handler a potential request for no space

		if	( !sz )
			sz	= 1;

		if	( blockLeft < sz )
		{
			//	Is the request greater than the largest buffer size ?

			if	( sz > memBlockSize )
				return	0;		// If it is, there is nothing we can do


			//	Allocate a new block

			Block *	pNewBlock	= rnew Block;


			//	Did the allocation succeed ?  If so connect it up

			if	( pNewBlock )
			{
				//	Handle the initial state

				if	( tail )
					tail	= tail->next	= pNewBlock;
				else
					head	= tail			= pNewBlock;

				//	Compute the remaining space

				blockLeft	= memBlockSize - sz;

			}	// End of IF then
			else
				return	0;		// Oh-oh!  Memory allocation failure

		}	// End of IF then
		else
			blockLeft	-= sz;	// Deduct the allocated amount

		//	And return the buffer address

		return	&( tail->memBlock[ blockLeft ]);

	}	// End of IF else
}	// End of "HeapManager" FUNCTION "getMemory(unsigned int,int)"




//	Friend functions of 'DName'

inline DName	__near __pascal	operator + ( char c, const DName & rd )
{	return	DName ( c ) + rd;	}

inline DName	__near __pascal	operator + ( DNameStatus st, const DName & rd )
{	return	DName ( st ) + rd;	}

inline DName	__near __pascal	operator + ( pcchar_t s, const DName & rd )
{	return	DName ( s ) + rd;	}


//	The 'DName' constructors

inline		__near	DName::DName ()					{	node	= 0;	stat	= DN_valid;	isIndir	= 0;	isAUDC	= 0; isAUDTThunk = 0;	NoTE	= 0; }
//!!!R inline		__near	DName::DName ( DNameNode * pd )	{	node	= pd;	stat	= DN_valid;	isIndir	= 0;	isAUDC	= 0; isAUDTThunk = 0;	NoTE	= 0; }

__near	DName::DName ( char c )
{
	stat	= DN_valid;
	isIndir	= 0;
	isAUDC	= 0;
	isAUDTThunk = 0;
	node	= 0;
	NoTE	= 0;

	//	The NULL character is boring, do not copy

	if	( c )
		doPchar ( &c, 1 );

}	// End of "DName" CONSTRUCTOR '(char)'


#if	1
inline __near	DName::DName ( const DName & rd )
{
	stat	= rd.stat;
	isIndir	= rd.isIndir;
	isAUDC	= rd.isAUDC;
	isAUDTThunk = rd.isAUDTThunk;
	node	= rd.node;
	NoTE	= rd.NoTE;
}	// End of "DName" CONSTRUCTOR '(const DName&)'
#endif


__near	DName::DName ( DName * pd )
{
	if	( pd )
	{
		node	= gnew pDNameNode ( pd );
		stat	= ( node ? DN_valid : DN_error );

	}	// End of IF else
	else
	{
		stat	= DN_valid;
		node	= 0;

	}	// End of IF else

	isIndir	= 0;
	isAUDC	= 0;
	isAUDTThunk = 0;
	NoTE	= 0;

}	// End of "DName" CONSTRUCTOR '( DName* )'


__near	DName::DName ( pcchar_t s )
{
	stat	= DN_valid;
	node	= 0;
	isIndir	= 0;
	isAUDC	= 0;
	isAUDTThunk = 0;
	NoTE	= 0;

	if	( s )
		doPchar ( s, (int)strlen ( s ));

}	// End of "DName" CONSTRUCTOR '(pcchar_t)'


__near	DName::DName ( pcchar_t & name, char terminator )
{
	stat	= DN_valid;
	isIndir	= 0;
	isAUDC	= 0;
	isAUDTThunk = 0;
	node	= 0;
	NoTE	= 0;

	//	Is there a string ?

	if	( name )
		if	( *name )
		{
			int	len	= 0;


			//	How long is the string ?

            pcchar_t s;
			for	( s = name; *name && ( *name != terminator ); name++ )
				if	( isValidIdentChar ( *name ) || UnDecorator::doNoIdentCharCheck () )
					len++;
				else
				{
					stat	= DN_invalid;

					return;

				}	// End of IF else

			//	Copy the name string fragment

			doPchar ( s, len );

			//	Now gobble the terminator if present, handle error conditions

			if	( *name )
			{
				if	( *name++ != terminator )
				{
					stat	= DN_error;
					node	= 0;

				}	// End of IF then
				else
					stat	= DN_valid;

			}	// End of IF then
			elif	( status () == DN_valid )
				stat	= DN_truncated;

		}	// End of IF then
		else
			stat	= DN_truncated;
	else
		stat	= DN_invalid;

}	// End of "DName" CONSTRUCTOR '(pcchar_t&,char)'


__near	DName::DName (__int64 num )
{
	char	buf[ 11 ];
	char *	pBuf	= buf + 10;
	bool bMinus = num < 0;
	if (bMinus)
		num = -num;

	stat	= DN_valid;
	node	= 0;
	isIndir	= 0;
	isAUDC	= 0;
	isAUDTThunk = 0;
	NoTE	= 0;

	//	Essentially, 'ultoa ( num, buf, 10 )' :-

	*pBuf	= 0;
		

	do
	{
		*( --pBuf )	= (char)(( num % 10 ) + '0' );
		num			/= 10UL;

	}	while	( num );
	if (bMinus)
		*--pBuf = '-';

	doPchar ( pBuf, ( 10 - (int) ( pBuf - buf )));

}	// End of "DName" CONSTRUCTOR '(unsigned long)'

__near	DName::DName ( unsigned __int64 num )
{
	char	buf[ 11 ];
	char *	pBuf	= buf + 10;


	stat	= DN_valid;
	node	= 0;
	isIndir	= 0;
	isAUDC	= 0;
	isAUDTThunk = 0;
	NoTE	= 0;

	//	Essentially, 'ultoa ( num, buf, 10 )' :-

	*pBuf	= 0;

	do
	{
		*( --pBuf )	= (char)(( num % 10 ) + '0' );
		num			/= 10UL;

	}	while	( num );

	doPchar ( pBuf, ( 10 - (int) ( pBuf - buf )));

}	// End of "DName" CONSTRUCTOR '(unsigned long)'


__near	DName::DName ( DNameStatus st )
{
	stat	= ((( st == DN_invalid ) || ( st == DN_error )) ? st : DN_valid );
	node	= gnew DNameStatusNode ( st );
	isIndir	= 0;
	isAUDC	= 0;
	isAUDTThunk = 0;
	NoTE	= 0;

	if	( !node )
		stat	= DN_error;

}	// End of "DName" CONSTRUCTOR '(DNameStatus)'



//	Now the member functions for 'DName'

int		__near	DName::isValid () const		{	return	(( status () == DN_valid ) || ( status () == DN_truncated ));	}
int		__near	DName::isEmpty () const		{	return	(( node == 0 ) || !isValid ());	}

inline	DNameStatus	__near	DName::status () const	{	return	(DNameStatus)stat;	}	// The cast is to keep Glockenspiel quiet

inline	DName &	DName::setPtrRef ()			{	isIndir	= 1;	return	*this;	}
inline	int		DName::isPtrRef () const	{	return	isIndir;	}
inline	int		DName::isUDC () const		{	return	( !isEmpty () && isAUDC );	}
inline	void	DName::setIsUDC ()			{	if	( !isEmpty ())	isAUDC	= TRUE;	}
inline	int		DName::isUDTThunk () const	{	return	( !isEmpty () && isAUDTThunk );	}
inline	void	DName::setIsUDTThunk ()		{	if	( !isEmpty ())	isAUDTThunk	= TRUE;	}
inline	int		DName::isArray () const		{	return isArrayType;	}
inline void		DName::setIsArray()			{	isArrayType = TRUE; }
inline	int		DName::isNoTE () const		{	return NoTE;	}
inline	void	DName::setIsNoTE ()			{	NoTE = TRUE;	}
inline	int		DName::isPinPtr() const		{	return pinPtr; }
inline	void	DName::setIsPinPtr()		{	pinPtr = TRUE; }
inline	int		DName::isComArray() const	{	return comArray; }
inline	void	DName::setIsComArray()		{	comArray = TRUE; }
inline	int		DName::isVCallThunk() const	{	return vcallThunk; }
inline	void	DName::setIsVCallThunk()	{	vcallThunk = TRUE; }


int	__near	DName::length () const
{
	int	len	= 0;


	if	( !isEmpty ())
		for	(const DNameNode * pNode = node; pNode; pNode = pNode->nextNode ())
			len	+= pNode->length ();

	return	len;

}	// End of "DName" FUNCTION "length"


char	__near	DName::getLastChar () const
{
	const DNameNode * pLast = 0;

	if ( !isEmpty ())
		for (const DNameNode * pNode = node; pNode; pNode = pNode->nextNode ())
			if ( pNode->length () != 0 )
				pLast = pNode;

	return	pLast != 0 ? pLast->getLastChar () : '\0';

}	// End of "DName" FUNCTION "getLastChar"

pchar_t	DName::getString(pchar_t buf, pchar_t end) const {
	if (!isEmpty())
		for (const DNameNode *cur = node; cur; cur=cur->nextNode())
			buf = cur->getString(buf, end);
	return buf;
}

pchar_t	DName::getString (pchar_t buf, int max) const {
	if (isEmpty()) {
		if (buf)
			*buf = 0;
	} else {
		//	Does the caller want a buffer allocated ?
		if	(!buf) {
			max	= length () + 1;
			buf	= gnew char[ max ];	// Get a buffer big enough
		}	// End of IF then

		//	If memory allocation failure, then return no buffer

		if (buf)
			*getString(buf, buf+max-1) = 0;
	}
	return	buf;
}

DName	__near	DName::operator + ( char ch ) const
{
	DName	local ( *this );


	if	( local.isEmpty ())
		local	= ch;
	else
		local	+= ch;

	//	And return the newly formed 'DName'

	return	local;

}	// End of "DName" OPERATOR "+(char)"


DName	__near	DName::operator + ( pcchar_t str ) const
{
	DName	local ( *this );


	if	( local.isEmpty ())
		local	= str;
	else
		local	+= str;

	//	And return the newly formed 'DName'

	return	local;

}	// End of "DName" OPERATOR "+(pcchar_t)"


DName	__near	DName::operator + ( const DName & rd ) const
{
	DName	local ( *this );


	if		( local.isEmpty ())
		local	= rd;
	elif	( rd.isEmpty ())
		local	+= rd.status ();
	else
		local	+= rd;

	//	And return the newly formed 'DName'

	return	local;

}	// End of "DName" OPERATOR "+(const DName&)"


DName	__near	DName::operator + ( DName * pd ) const
{
	DName	local ( *this );


	if	( local.isEmpty ())
		local	= pd;
	else
		local	+= pd;

	//	And return the newly formed 'DName'

	return	local;

}	// End of "DName" OPERATOR "+(DName*)"


DName	__near	DName::operator + ( DNameStatus st ) const
{
	DName	local ( *this );


	if	( local.isEmpty ())
		local	= st;
	else
		local	+= st;

	//	And return the newly formed 'DName'

	return	local;

}	// End of "DName" OPERATOR "+(DNameStatus)"

void DName::append(const DNameNode *newRight) {
	if (newRight) {
		*node += newRight;
	} else
		stat = DN_error;
}

DName &	__near	DName::operator += ( char ch )
{
	if	( ch )
		if	( isEmpty ())
			*this	= ch;
		else
		{
//!!!R			node	= node->clone ();

			append(gnew charNode(ch));

		}	// End of IF

	//	And return self

	return	*this;

}	// End of "DName" OPERATOR "+=(char)"


DName &DName::operator+=(pcchar_t str) {
	if (isValid() && str && *str) {
		if (isEmpty())
			*this = str;
		else
			append(gnew pcharNode(str, und_strlen(str)));
	}
	return *this;
}

DName & DName::operator+=(const DName &rd) {
	if (isValid()) {
		if	(rd.isEmpty ())
			*this	+= rd.status ();
		else if	( isEmpty ())
			*this = rd;
		else
			append(rd.node);
	}
	return	*this;
}

DName &	__near	DName::operator += ( DName * pd )
{
	if	( pd )
		if		( isEmpty ())
			*this	= pd;
		elif	(( pd->status () == DN_valid ) || ( pd->status () == DN_truncated ))
		{
			append(gnew pDNameNode(pd));

		}	// End of IF then
		else
			*this	+= pd->status ();

	//	And return self

	return	*this;

}	// End of "DName" OPERATOR "+=(DName*)"


DName &	__near	DName::operator += ( DNameStatus st )
{
	if	( isEmpty () || (( st == DN_invalid ) || ( st == DN_error )))
		*this	= st;
	else
		append(DNameStatusNode::make(st));
	return	*this;
}

DName &	__near	DName::operator |= ( const DName & rd )
{
	//	Attenuate the error status.  Always becomes worse.  Don't propogate truncation

	if	(( status () != DN_error ) && !rd.isValid ())
		stat	= rd.status ();

	//	And return self

	return	*this;

}	// End of "DName" OPERATOR '|=(const DName&)'



DName &	__near	DName::operator = ( char ch )
{
	isIndir	= 0;
	isAUDC	= 0;
	isAUDTThunk = 0;

	doPchar ( &ch, 1 );

	return	*this;

}	// End of "DName" OPERATOR '=(char)'


DName &	__near	DName::operator = ( pcchar_t str )
{
	isIndir	= 0;
	isAUDC	= 0;
	isAUDTThunk = 0;

	doPchar ( str, (int)strlen ( str ));

	//	And return self

	return	*this;

}	// End of "DName" OPERATOR '=(pcchar_t)'


DName &	__near	DName::operator = ( const DName & rd )
{
	if	(( status () == DN_valid ) || ( status () == DN_truncated ))
	{
		stat	= rd.stat;
		isIndir	= rd.isIndir;
		isAUDC	= rd.isAUDC;
		isAUDTThunk = rd.isAUDTThunk;
		node	= rd.node;

	}	// End of IF

	//	And return self

	return	*this;

}	// End of "DName" OPERATOR '=(const DName&)'


DName &	__near	DName::operator = ( DName * pd )
{
	if	(( status () == DN_valid ) || ( status () == DN_truncated ))
		if	( pd )
		{
			isIndir	= 0;
			isAUDC	= 0;
			isAUDTThunk = 0;
			node	= gnew pDNameNode ( pd );

			if	( !node )
				stat	= DN_error;

		}	// End of IF then
		else
			*this	= DN_error;

	//	And return self

	return	*this;

}	// End of "DName" OPERATOR '=(DName*)'


DName &	__near	DName::operator = ( DNameStatus st )
{
	if	(( st == DN_invalid ) || ( st == DN_error ))
	{
		node	= 0;

		if	( status () != DN_error )
			stat	= st;

	}	// End of IF then
	elif	(( status () == DN_valid ) || ( status () == DN_truncated ))
	{
		isIndir	= 0;
		isAUDC	= 0;
		isAUDTThunk = 0;
		node	= gnew DNameStatusNode ( st );

		if	( !node )
			stat	= DN_error;

	}	// End of ELIF then

	//	And return self

	return	*this;

}	// End of "DName" OPERATOR '=(DNameStatus)'


//	Private implementation functions for 'DName'

void	__near	DName::doPchar ( pcchar_t str, int len )
{
	if	( !(( status () == DN_invalid ) || ( status () == DN_error )))
		if		( node )
			*this	= DN_error;
		elif	( str && len )
		{
			//	Allocate as economically as possible

			switch	( len )
			{
			case 0:
					stat	= DN_error;
				break;

			case 1:
					node	= gnew charNode ( *str );

					if	( !node )
						stat	= DN_error;
				break;

			default:
					node	= gnew pcharNode ( str, len );

					if	( !node )
						stat	= DN_error;
				break;

			}	// End of SWITCH
		}	// End of ELIF
		else
			stat	= DN_invalid;

}	// End of "DName" FUNCTION "doPchar(pcchar_t,int)"



//	The member functions for the 'Replicator'

inline	int	__near	Replicator::isFull () const		{	return	( index == 9 );	}
inline	__near		Replicator::Replicator ()
//!!!R :	ErrorDName ( DN_error ), InvalidDName ( DN_invalid )
{	index	= -1;	}



Replicator &	__near	Replicator::operator += ( const DName & rd )
{
	if	( !isFull () && !rd.isEmpty ())
	{
		DName *	pNew	= gnew DName ( rd );


		//	Don't update if failed

		if	( pNew )
			dNameBuffer[ ++index ]	= pNew;

	}	// End of IF

	return	*this;

}	// End of "Replicator" OPERATOR '+=(const DName&)'


DName Replicator::operator [] ( int x ) const
{
	if		(( x < 0 ) || ( x > 9 ))
		return	DN_error;
	elif	(( index == -1 ) || ( x > index ))
		return	DN_invalid;
	else
		return	*dNameBuffer[ x ];

}	// End of "Replicator" OPERATOR '[](int)'



//	The member functions for the 'DNameNode' classes

#if	( !NO_VIRTUAL )
__near	DNameNode::DNameNode ()
#else	// } elif NO_VIRTUAL {
__near	DNameNode::DNameNode ( NodeType ndTy )
:	typeIndex ( ndTy )
#endif	// NO_VIRTUAL
{	next	= 0;	}

inline	const DNameNode *	__near	DNameNode::nextNode () const		{	return	next;	}


/*!!!
DNameNode *	__near	DNameNode::clone ()
{
	return	gnew pDNameNode ( gnew DName ( this ));
}*/

#if	( NO_VIRTUAL )
int	__near	DNameNode::length () const
{	//	Pure function, should not be called

	switch	( typeIndex )
	{
	case charNode_t:
		return	((charNode*)this )->length ();

	case pcharNode_t:
		return	((pcharNode*)this )->length ();

	case pDNameNode_t:
		return	((pDNameNode*)this )->length ();

	case DNameStatusNode_t:
		return	((DNameStatusNode*)this )->length ();

	}	// End of SWITCH

	return	0;
}


int	__near	DNameNode::getLastChar () const
{	//	Pure function, should not be called

	switch	( typeIndex )
	{
	case charNode_t:
		return	((charNode*)this )->getLastChar ();

	case pcharNode_t:
		return	((pcharNode*)this )->getLastChar ();

	case pDNameNode_t:
		return	((pDNameNode*)this )->getLastChar ();

	case DNameStatusNode_t:
		return	((DNameStatusNode*)this )->getLastChar ();

	}	// End of SWITCH

	return	0;
}


pchar_t	__near	DNameNode::getString ( pchar_t s, int l ) const
{	//	Pure function, should not be called

	switch	( typeIndex )
	{
	case charNode_t:
		return	((charNode*)this )->getString ( s, l );

	case pcharNode_t:
		return	((pcharNode*)this )->getString ( s, l );

	case pDNameNode_t:
		return	((pDNameNode*)this )->getString ( s, l );

	case DNameStatusNode_t:
		return	((DNameStatusNode*)this )->getString ( s, l );

	}	// End of SWITCH

	return	0;
}
#endif	// NO_VIRTUAL


const DNameNode &	__near	DNameNode::operator += (const DNameNode * pNode ) const
{
	if	( pNode )
	{
		if	( next )
		{
			//	Skip to the end of the chain

            DNameNode* pScan;
			for	( pScan = (DNameNode*)next; pScan->next; pScan = (DNameNode*)pScan->next )
				;

			//	And append the new node

			pScan->next	= (DNameNode*)pNode;

		}	// End of IF then
		else
			next	= (DNameNode*)pNode;

	}	// End of IF

	//	And return self

	return	*this;

}	// End of "DNameNode" OPERATOR '+=(DNameNode*)'




//	The 'pcharNode' virtual functions

__near	pcharNode::pcharNode ( pcchar_t str, size_t len )
#if ( NO_VIRTUAL )
:	DNameNode ( pcharNode_t )
#endif	// NO_VIRTUAL
{
	//	Get length if not supplied

	if	( !len && str )
		len	= strlen ( str );

	//	Allocate a new string buffer if valid state

	if	( len && str )
	{
		me		= gnew char[ len ];
		myLen	= len;

		if	( me )
			strncpy ( me, str, len );

	}	// End of IF then
	else
	{
		me		= 0;
		myLen	= 0;

	}	// End of IF else
}	// End of "pcharNode" CONSTRUCTOR '(pcchar_t,int)'




unsigned und_strlen (pcchar_t p) {
	return (unsigned)strlen(p);
}

void und_memcpy(pchar_t dst, pcchar_t src, unsigned int len) {
	memcpy(dst, src, len);
}

unsigned int und_strncmp ( pcchar_t x, pcchar_t y, unsigned int n) {
	return strncmp(x, y, n);
}

