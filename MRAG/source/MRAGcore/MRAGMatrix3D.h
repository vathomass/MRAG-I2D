/*
 *  Matrix3D.h
 *  ReactionDiffusion
 *
 *  Created by Diego Rossinelli on 10/19/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */
#include <assert.h>
#pragma once
namespace MRAG
{
	
template <class DataType, bool bPrimitiveType=false, template <typename T> class allocator=std::allocator>  
class Matrix3D
{
private:
	DataType * m_pData;
	unsigned int m_vSize[3];
	unsigned int m_nElements;
	unsigned int m_nElementsPerSlice;

public:
	void _Release()
	{
		if (m_pData != NULL)
		{
			allocator<DataType> alloc;
			
			if (!bPrimitiveType)
			{
				for(int i=0; i<m_nElements; i++)
#if TBB_OLD_VERSION
					alloc.destroy(m_pData+i);
#else
					std::allocator_traits<allocator<DataType>>::destroy(alloc, m_pData+i);
#endif
			}
			
#if TBB_OLD_VERSION
			alloc.deallocate(m_pData, m_nElements);
#else		
			std::allocator_traits<allocator<DataType>>::deallocate(alloc, m_pData, m_nElements);
#endif
			m_pData = NULL;
		}
	}
	
	void _Setup(unsigned int nSizeX, unsigned int nSizeY, unsigned int nSizeZ)
	{
		_Release();
		
		m_vSize[0] = nSizeX;
		m_vSize[1] = nSizeY;
		m_vSize[2] = nSizeZ;
		
		m_nElementsPerSlice = nSizeX*nSizeY;
		
		m_nElements = nSizeX*nSizeY*nSizeZ;
		
		allocator<DataType> alloc;
#if TBB_OLD_VERSION
		m_pData = alloc.allocate(m_nElements);
#else
		using alloc_traits = std::allocator_traits<allocator<DataType>>;
		m_pData = alloc_traits::allocate(alloc, m_nElements);
#endif

		assert(m_pData != NULL);
		
		if (!bPrimitiveType)
		{
			for(int i=0; i<m_nElements; i++)
#if TBB_OLD_VERSION
				alloc.construct(m_pData+i, DataType());
#else
				alloc_traits::construct(alloc, m_pData + i, DataType());
#endif
		}
	}

	
	~Matrix3D() { _Release(); }
	

	Matrix3D(const Matrix3D& m):
	m_pData(NULL), m_nElements(0), m_nElementsPerSlice(0)
	{
		m_vSize[0] = m.m_vSize[0];
		m_vSize[1] = m.m_vSize[1];
		m_vSize[2] = m.m_vSize[2];
		
		m_nElementsPerSlice = m_vSize[0]*m_vSize[1];
		
		m_nElements = m_vSize[0]*m_vSize[1]*m_vSize[2];

		m_pData = m.m_pData;
		/*_Setup(m.m_vSize[0], m.m_vSize[1], m.m_vSize[2]);
		
		const int n = m_nElements;
		for(int i=0; i<n; i++)
			m_pData[i] = m.m_pData[i];*/
	}
	
	Matrix3D& operator=(const Matrix3D& m)
	{
		assert(m_vSize[0] == m.m_vSize[0]);
		assert(m_vSize[1] == m.m_vSize[1]);
		assert(m_vSize[2] == m.m_vSize[2]);
		
		const int n = m_nElements;
		for(int i=0; i<n; i++)
			m_pData[i] = m.m_pData[i];
		
		return *this;
	}
	
		
	Matrix3D(unsigned int nSizeX, unsigned int nSizeY, unsigned int nSizeZ):
	m_pData(NULL),
	m_nElements(0),
	m_nElementsPerSlice(0)
	{
		_Setup(nSizeX,nSizeY,nSizeZ);
	}
	
	Matrix3D():
	m_pData(NULL),
	m_nElements(-1),
	m_nElementsPerSlice(-1)
	{
	}
	
	Matrix3D(FILE * f, bool bSwapBytes):
	m_pData(NULL),
	m_nElements(0),
	m_nElementsPerSlice(0)
	{
		Deserialize(f, bSwapBytes);
	}
	
	
	inline DataType& Access(unsigned int ix, unsigned int iy, unsigned int iz) const
	{
#ifndef NDEBUG
		assert(ix<m_vSize[0]);
		assert(iy<m_vSize[1]);
		assert(iz<m_vSize[2]);
#endif
		
		return m_pData[iz*m_nElementsPerSlice + iy*m_vSize[0] + ix];
	}
	
	inline const DataType& Read(unsigned int ix, unsigned int iy, unsigned int iz) const
	{
#ifndef NDEBUG
		assert(ix<m_vSize[0]);
		assert(iy<m_vSize[1]);
		assert(iz<m_vSize[2]);
#endif
		
		return m_pData[iz*m_nElementsPerSlice + iy*m_vSize[0] + ix];
	}
	
	
	inline DataType& LinAccess(unsigned int i) const
	{
#ifndef NDEBUG
		assert(i<m_nElements);
#endif
		return m_pData[i];
	}
	
	inline unsigned int  getNumberOfElements() const
	{
		return m_nElements;
	}
	
	inline unsigned int * getSize() const
	{
		return (unsigned int *)m_vSize;
	}
	
	inline Matrix3D& operator=(DataType d)
	{
		for(int i=0;i<m_nElements; i++) m_pData[i] = d;
		
		return *this;
	}
	
	void  Serialize(FILE * f)
	{
		fwrite((void*) this, sizeof(Matrix3D<DataType>), 1, f);
		fwrite((void*) m_pData, sizeof(DataType), m_nElements, f);
	}
	
	void  Deserialize(FILE *f, bool bSwapBytes)
	{
		{
			const unsigned int nIntMembers = 1+3+1+1; 
			unsigned int buf[nIntMembers];
			fread((void *)buf, 4,nIntMembers, f);
			if(bSwapBytes) abort();
			m_pData = NULL;
			m_vSize[0] = buf[1];
			m_vSize[1] = buf[2];
			m_vSize[2] = buf[3];
			m_nElements = buf[4];
			m_nElementsPerSlice = buf[5];
		}
		
		m_pData = NULL;
		_Setup(m_vSize[0], m_vSize[1], m_vSize[2]);
		
		if (bSwapBytes)
		{
			abort();
		}
		else
			fread((void*) m_pData, sizeof(DataType), m_nElements, f);
	}
	
};



template <class DataType> inline void SwapBytes(unsigned char * pBuffer, int nBufferSize)
{
	unsigned char * ptr;
	unsigned char tmp[sizeof(DataType)];
	int i,j;

	for(i=0, ptr=pBuffer; i<nBufferSize; i+=sizeof(DataType), ptr+=sizeof(DataType))
	{
		for(j=0;j<sizeof(DataType); j++)
			tmp[sizeof(DataType)-(j+1)] = ptr[j];

		for(j=0;j<sizeof(DataType); j++)
			 ptr[j] = tmp[j];
	}

	assert(i == nBufferSize); // se no son cazzi acidi
}


}

