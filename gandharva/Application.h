#ifndef _4c82d541_2c58_433d_b91b_4cac673cb9fb
#define _4c82d541_2c58_433d_b91b_4cac673cb9fb

#include "MyWindows.h"

namespace gandharva
{

namespace Application
{
	// ���b�Z�[�W���[�v���܂킷
	UINT Run();

	// �������[�v������Ƃ��A���̍ŏ��ɂ����u���Ƃ���
	void PeekAndDispatchMessage();

	// �w�肳�ꂽ���b�Z�[�W���L���[�����菜��
	// (������� dispatch ���s��)
	void FlushMessageQueue(UINT msgMin, UINT msgMax);
}

} // namespace gandharva
#endif//_4c82d541_2c58_433d_b91b_4cac673cb9fb
