/*
 * =====================================================================================
 *
 *       Filename:  paragraphvector.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  07/04/2014 12:12:58 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  Xiansheng Wang,
 *   Organization:  B5M
 *
 * =====================================================================================
 */

#include <stdio.h>
#include "idmlib/para2vec/paragraphvec.h"
extern void setParameter(const ModelArgument& ma);
extern bool initML(bool bPredict);
extern void TrainMLModel();
extern std::vector<float> Predict(const std::string& para);

ParagraphVector::ParagraphVector(const ModelArgument& ma): ma_(ma)
{

}

ParagraphVector::~ParagraphVector()
{

}


bool ParagraphVector::Init( bool bPredict )
{
    if (!bPredict)
    {
        setParameter(ma_);
    }
    return initML(bPredict);
}


void ParagraphVector::TrainModel()
{

    TrainMLModel();
}

std::vector<float> ParagraphVector::PredictPara( const std::string& para )
{
    return Predict(para);
}
