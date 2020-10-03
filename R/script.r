## 获取参数
args<-commandArgs(T)
## 参数列表
argumentsFile=args[1]
dataOutput=args[2]
## 读数据
arguments=read.csv(argumentsFile,sep = ",",header = FALSE)

## 拆分数据
WSkk=arguments$V1
RHkk=arguments$V2
AODkk=arguments$V3
Timekk=arguments$V4
datalen=length(AODkk)

## 加载模型
modelDir="./R/aodModel.RData"
load(modelDir)

# ## 预测输出

result=array(data=0.0,dim = datalen)
i=1
for (cc in 1:datalen){
  if(AODkk[cc]<=0|WSkk[cc]<=0|RHkk[cc]<=0){
    next
  }
  result[cc]=predict(aodModel,data.frame(Time=Timekk[cc],AOD=AODkk[cc],WS=WSkk[cc],RH=RHkk[cc]))
}
resultPM2_5=data.frame(round(result))
write.table(resultPM2_5,dataOutput,row.names = FALSE,col.names =FALSE)

# dataTimeFile=args[1]
# dataAODFile=args[2]
# dataWSFile=args[3]
# dataRHFile=args[4]
# dataOutput=args[5]

# dataTimeFile="./tmp/time20150110.csv"
# dataAODFile="./tmp/imageData.csv"
# dataWSFile="./tmp/WSdata20150110.csv"
# dataRHFile="./tmp/RHdata20150110.csv"
# dataOutput="./tmp/result.csv"

## 读取文件内容
# 假设csv文件第一行是第一条数据，即没有标头
# dataTime=read.csv(dataTimeFile,sep = ",",header = FALSE)
# dataAOD=read.csv(dataAODFile,sep = ",",header = FALSE)
# dataWS=read.csv(dataWSFile,sep = ",",header = FALSE)
# dataRH=read.csv(dataRHFile,sep = ",",header = FALSE)
# # 加载模型
# modelDir="aodModel.RData"
# load(modelDir)


