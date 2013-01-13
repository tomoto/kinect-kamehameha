#include "TrigonometricTable.h"
#include "math.h"

TrigonometricTable::TrigonometricTable(int slices)
{
	m_slices = slices;
	m_sins = new float[slices];
	m_coss = new float[slices];
	float step = float(M_PI * 2 / slices);

	for (int i = 0; i < slices; i++) {
		m_sins[i] = ::sinf(step * i);
		m_coss[i] = ::cosf(step * i);
	}
}

TrigonometricTable::~TrigonometricTable()
{
	delete[] m_sins;
	delete[] m_coss;
}
