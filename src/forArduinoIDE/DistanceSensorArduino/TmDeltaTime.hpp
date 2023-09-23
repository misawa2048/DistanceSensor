#ifndef __TMDELTATIME_H__
#define __TMDELTATIME_H__
#include <Arduino.h>

/*** TmDeltaTime
 * 一定時間ごとにコールバックを返すクラス ***/
class TmDeltaTime{
    public:
    TmDeltaTime(){
      m_trigWorkArr=NULL;
    };
    ~TmDeltaTime(){
      if(m_trigWorkArr!=NULL){
        free(m_trigWorkArr);
        m_trigWorkArr=NULL;
      }
    };

    /***
     * Setup セットアップ内で呼ぶ
     *  _maxTrig:最大タイマートリガ数(デフォルト:8)    ***/
    void Setup(int8_t _maxTrig=8){
      m_trigNum = (_maxTrig>0)?_maxTrig:1;
      m_trigWorkArr = (TrigWork*)malloc(sizeof(TrigWork)*m_trigNum);
      for(int i=0;i<m_trigNum;++i){
        m_trigWorkArr[i].func=NULL;
        m_trigWorkArr[i].timer=0;
      }
      m_currentMillis = millis();
      m_deltaMillis = 0;
    }

    /*** 
     * AddTrig 一定時間ごとに呼ばれる関数を追加
     * _pFunc:コールバック関数
     * uint32_t: コールバック関数が呼ばれる周期[ミリ秒]
     * 返値: 成功でトリガーId,失敗で-1    ***/
    int8_t AddTrig(void (*_pFunc)(uint32_t), uint32_t _trigTime){
      for(int8_t trigId=0;trigId<m_trigNum;++trigId){
        if(m_trigWorkArr[trigId].func==NULL){
          m_trigWorkArr[trigId].func = _pFunc;
          m_trigWorkArr[trigId].trigTime = _trigTime;
          m_trigWorkArr[trigId].timer = 0;
          return trigId;
        }
      }
      return -1;
    }

    /** RemoveTrig 一定時間ごとに呼ばれる関数を削除
     * _trigId:コールバック関数
     * 返り値: 成功したらtrue   ***/
     bool RemoveTrig(int8_t _trigId){
      if(m_trigWorkArr[_trigId].func!=NULL){
        m_trigWorkArr[_trigId].func = NULL;
        m_trigWorkArr[_trigId].trigTime = 0;
        m_trigWorkArr[_trigId].timer = 0;
        return true;
      }
      return false;
    }

    /*** Update メインループ内で呼ぶ
     * 返り値: 前回呼ばれてからの時間[ミリ秒] ***/
    uint32_t Update(){
        uint32_t nowMillis = millis();
        m_deltaMillis = nowMillis - m_currentMillis;
        m_currentMillis = nowMillis;
        for(int8_t i=0;i<m_trigNum;++i){
          if(m_trigWorkArr[i].func!=NULL){
            m_trigWorkArr[i].timer += m_deltaMillis;
            if(m_trigWorkArr[i].timer >= m_trigWorkArr[i].trigTime){
              m_trigWorkArr[i].func(m_trigWorkArr[i].timer);
              m_trigWorkArr[i].timer -= m_trigWorkArr[i].trigTime;
            }
          }
        }
        return m_deltaMillis;
    };

    /*** GetRemainTrigNum 使用できる残トリガー数を取得
     * 返り値: 使用できる残トリガー数 ***/
    int8_t GetRemainTrigNum(){
        int8_t cnt = 0;
        for(int8_t i=0;i<m_trigNum;++i)
          if(m_trigWorkArr[i].func!=NULL)
            cnt++;
        return cnt;
    }
    private:
    struct TrigWork{
      void (*func)(uint32_t);
      uint32_t trigTime;
      uint32_t timer;
    } _TrigWork;

    TrigWork* m_trigWorkArr;
    int8_t m_trigNum;
    uint32_t m_currentMillis;
    uint32_t m_deltaMillis;
};
#endif // __TMDELTATIME_H__
