/*****************************************************************************************************************************
This project was supported by the National Basic Research 973 Program of China under Grant No.2011CB302301
Huazhong University of Science and Technology (HUST)   Wuhan National Laboratory for Optoelectronics

FileName： initialize.c
Author: Hu Yang		Version: 2.1	Date:2011/12/02
Description: 

History:
<contributor>     <time>        <version>       <desc>                   <e-mail>
Yang Hu	        2009/09/25	      1.0		    Creat SSDsim       yanghu@foxmail.com
                2010/05/01        2.x           Change 
Zhiming Zhu     2011/07/01        2.0           Change               812839842@qq.com
Shuangwu Zhang  2011/11/01        2.1           Change               820876427@qq.com
Chao Ren        2011/07/01        2.0           Change               529517386@qq.com
Hao Luo         2011/01/01        2.0           Change               luohao135680@gmail.com
Zhiming Zhu     2012/07/19        2.1.1         Correct erase_planes()   812839842@qq.com  
*****************************************************************************************************************************/
#define _CRT_SECURE_NO_WARNINGS
#define _CRTDBG_MAP_ALLOC
 
#include <stdlib.h>
#include <crtdbg.h>
#include "initialize.h"
#include "pagemap.h"

#define FALSE		0
#define TRUE		1

#define ACTIVE_FIXED 0
#define ACTIVE_ADJUST 1
#define DATABASE_SIZE 100000

#define REQUEST_QUEUE_LENGTH 1024

/************************************************************************
* Compare function for AVL Tree                                        
************************************************************************/
extern int keyCompareFunc(TREE_NODE *p , TREE_NODE *p1)
{
	struct buffer_group *T1=NULL,*T2=NULL;

	T1=(struct buffer_group*)p;
	T2=(struct buffer_group*)p1;


	if(T1->group< T2->group) return 1;
	if(T1->group> T2->group) return -1;

	return 0;
}


extern int freeFunc(TREE_NODE *pNode)
{
	
	if(pNode!=NULL)
	{
		free((void *)pNode);
	}
	
	
	pNode=NULL;
	return 1;
}


/**********   initiation   ******************
*modify by zhouwen
*November 08,2011
*initialize the ssd struct to simulate the ssd hardware
*1.this function allocate memory for ssd structure 
*2.set the infomation according to the parameter file
*******************************************/
struct ssd_info *initiation(struct ssd_info *ssd)
{
	unsigned int x=0,y=0,i=0,j=0,k=0,l=0,m=0,n=0;
	errno_t err;
	char buffer[300];
	struct parameter_value *parameters;
	FILE *fp=NULL;
	
	printf("input parameter file name:");
//	gets(ssd->parameterfilename);
 	strcpy_s(ssd->parameterfilename,16,"page.parameters");

	printf("\ninput trace file name:");
//	gets(ssd->tracefilename);
	strcpy_s(ssd->tracefilename,25,"home-sample-merged.ascii");
//	strcpy_s(ssd->tracefilename,25,"CFS.ascii");
//	strcpy_s(ssd->tracefilename,25,"DevDivRelease.ascii");

	printf("\ninput output file name:");
	//gets(ssd->outputfilename);
	strcpy_s(ssd->outputfilename,7,"o.txt");
//	strcpy_s(ssd->outputfilename,25,"CFS.out");
//	strcpy_s(ssd->outputfilename,25,"DevDivRelease.out");

	printf("\ninput statistic file name:");
	//gets(ssd->statisticfilename);
	strcpy_s(ssd->statisticfilename,16,"s.txt");
//	strcpy_s(ssd->statisticfilename,16,"CFS.dat");
//	strcpy_s(ssd->statisticfilename,25,"DevDivRelease.dat");

	//strcpy_s(ssd->statisticfilename2 ,16,"statistic2.dat");


	//导入ssd的配置文件
	parameters=load_parameters(ssd->parameterfilename);
	ssd->parameter=parameters;
	ssd->min_lsn=0x7fffffff;
	ssd->page=ssd->parameter->chip_num*ssd->parameter->die_chip*ssd->parameter->plane_die*ssd->parameter->block_plane*ssd->parameter->page_block;

	//初始化 dram
	ssd->dram = (struct dram_info *)malloc(sizeof(struct dram_info));
	alloc_assert(ssd->dram,"ssd->dram");
	memset(ssd->dram,0,sizeof(struct dram_info));
	initialize_dram(ssd);

	//初始化通道
	ssd->channel_head=(struct channel_info*)malloc(ssd->parameter->channel_number * sizeof(struct channel_info));
	alloc_assert(ssd->channel_head,"ssd->channel_head");
	memset(ssd->channel_head,0,ssd->parameter->channel_number * sizeof(struct channel_info));
	initialize_channels(ssd);
	
	//初始化页内容数据库
	ssd->pageDatabase = create_hash_table(DATABASE_SIZE);


	printf("\n");
	if((err=fopen_s(&ssd->outputfile,ssd->outputfilename,"w")) != 0)
	{
		printf("the output file can't open\n");
		return NULL;
	}

	printf("\n");
	if((err=fopen_s(&ssd->statisticfile,ssd->statisticfilename,"w"))!=0)
	{
		printf("the statistic file can't open\n");
		return NULL;
	}


	printf("\n");
// 	if((err=fopen_s(&ssd->statisticfile2,ssd->statisticfilename2,"w"))!=0)
// 	{
// 		printf("the second statistic file can't open\n");
// 		return NULL;
// 	}

	fprintf(ssd->outputfile,"parameter file: %s\n",ssd->parameterfilename); 
	fprintf(ssd->outputfile,"trace file: %s\n",ssd->tracefilename);
	fprintf(ssd->statisticfile,"parameter file: %s\n",ssd->parameterfilename); 
	fprintf(ssd->statisticfile,"trace file: %s\n",ssd->tracefilename);
	
	fflush(ssd->outputfile);
	fflush(ssd->statisticfile);

	if((err=fopen_s(&fp,ssd->parameterfilename,"r"))!=0)
	{
		printf("\nthe parameter file can't open!\n");
		return NULL;
	}

	//fp=fopen(ssd->parameterfilename,"r");

	fprintf(ssd->outputfile,"-----------------------parameter file----------------------\n");
	fprintf(ssd->statisticfile,"-----------------------parameter file----------------------\n");
	while(fgets(buffer,300,fp))
	{
		fprintf(ssd->outputfile,"%s",buffer);
		fflush(ssd->outputfile);
		fprintf(ssd->statisticfile,"%s",buffer);
		fflush(ssd->statisticfile);
	}

	fprintf(ssd->outputfile,"\n");
	fprintf(ssd->outputfile,"-----------------------simulation output1----------------------\n");
	fflush(ssd->outputfile);

	fprintf(ssd->statisticfile,"\n");
	fprintf(ssd->statisticfile,"-----------------------simulation output1----------------------\n");
	fflush(ssd->statisticfile);

	fclose(fp);
	printf("initiation is completed!\n");

	return ssd;
}


struct dram_info * initialize_dram(struct ssd_info * ssd)
{
	unsigned int page_num;

	struct dram_info *dram=ssd->dram;
	dram->dram_capacity = ssd->parameter->dram_capacity;		
	dram->buffer = (tAVLTree *)avlTreeCreate((void*)keyCompareFunc , (void *)freeFunc);
	dram->buffer->max_buffer_sector=ssd->parameter->dram_capacity/SECTOR; //512

	dram->map = (struct map_info *)malloc(sizeof(struct map_info));
	alloc_assert(dram->map,"dram->map");
	memset(dram->map,0, sizeof(struct map_info));

	page_num = ssd->parameter->page_block*ssd->parameter->block_plane*ssd->parameter->plane_die*ssd->parameter->die_chip*ssd->parameter->chip_num;

	dram->map->map_entry = (struct entry *)malloc(sizeof(struct entry) * page_num); //每个物理页和逻辑页都有对应关系
	alloc_assert(dram->map->map_entry,"dram->map->map_entry");
	memset(dram->map->map_entry,0,sizeof(struct entry) * page_num);
	
	return dram;
}

struct page_info * initialize_page(struct page_info * p_page )
{
	p_page->valid_state =0;
	p_page->free_state = PG_SUB;
	p_page->lpn = -1;
	p_page->written_count=0;
	return p_page;
}

struct blk_info * initialize_block(struct blk_info * p_block,struct parameter_value *parameter)
{
	unsigned int i;
	struct page_info * p_page;
	
	p_block->free_page_num = parameter->page_block;	// all pages are free
	p_block->last_write_page = -1;	// no page has been programmed

	p_block->page_head = (struct page_info *)malloc(parameter->page_block * sizeof(struct page_info));
	alloc_assert(p_block->page_head,"p_block->page_head");
	memset(p_block->page_head,0,parameter->page_block * sizeof(struct page_info));

	for(i = 0; i<parameter->page_block; i++)
	{
		p_page = &(p_block->page_head[i]);
		initialize_page(p_page );
	}
	return p_block;

}

struct plane_info * initialize_plane(struct plane_info * p_plane,struct parameter_value *parameter )
{
	unsigned int i;
	struct blk_info * p_block;
	p_plane->add_reg_ppn = -1;  //plane 里面的额外寄存器additional register -1 表示无数据
	p_plane->free_page=parameter->block_plane*parameter->page_block;

	p_plane->blk_head = (struct blk_info *)malloc(parameter->block_plane * sizeof(struct blk_info));
	alloc_assert(p_plane->blk_head,"p_plane->blk_head");
	memset(p_plane->blk_head,0,parameter->block_plane * sizeof(struct blk_info));

	for(i = 0; i<parameter->block_plane; i++)
	{
		p_block = &(p_plane->blk_head[i]);
		initialize_block( p_block ,parameter);			
	}
	return p_plane;
}

struct die_info * initialize_die(struct die_info * p_die,struct parameter_value *parameter,long long current_time )
{
	unsigned int i;
	struct plane_info * p_plane;

	p_die->token=0;
		
	p_die->plane_head = (struct plane_info*)malloc(parameter->plane_die * sizeof(struct plane_info));
	alloc_assert(p_die->plane_head,"p_die->plane_head");
	memset(p_die->plane_head,0,parameter->plane_die * sizeof(struct plane_info));

	for (i = 0; i<parameter->plane_die; i++)
	{
		p_plane = &(p_die->plane_head[i]);
		initialize_plane(p_plane,parameter );
	}

	return p_die;
}

struct chip_info * initialize_chip(struct chip_info * p_chip,struct parameter_value *parameter,long long current_time )
{
	unsigned int i;
	struct die_info *p_die;
	
	p_chip->current_state = CHIP_IDLE;
	p_chip->next_state = CHIP_IDLE;
	p_chip->current_time = current_time;
	p_chip->next_state_predict_time = 0;			
	p_chip->die_num = parameter->die_chip;
	p_chip->plane_num_die = parameter->plane_die;
	p_chip->block_num_plane = parameter->block_plane;
	p_chip->page_num_block = parameter->page_block;
	p_chip->subpage_num_page = parameter->subpage_page;
	p_chip->ers_limit = parameter->ers_limit;
	p_chip->token=0;
	p_chip->ac_timing = parameter->time_characteristics;		
	p_chip->read_count = 0;
	p_chip->program_count = 0;
	p_chip->erase_count = 0;

	p_chip->die_head = (struct die_info *)malloc(parameter->die_chip * sizeof(struct die_info));
	alloc_assert(p_chip->die_head,"p_chip->die_head");
	memset(p_chip->die_head,0,parameter->die_chip * sizeof(struct die_info));

	for (i = 0; i<parameter->die_chip; i++)
	{
		p_die = &(p_chip->die_head[i]);
		initialize_die( p_die,parameter,current_time );	
	}

	return p_chip;
}

struct ssd_info * initialize_channels(struct ssd_info * ssd )
{
	unsigned int i,j;
	struct channel_info * p_channel;
	struct chip_info * p_chip;

	// set the parameter of each channel
	for (i = 0; i< ssd->parameter->channel_number; i++)
	{
		p_channel = &(ssd->channel_head[i]);
		p_channel->chip = ssd->parameter->chip_channel[i];
		p_channel->current_state = CHANNEL_IDLE;
		p_channel->next_state = CHANNEL_IDLE;
		
		p_channel->chip_head = (struct chip_info *)malloc(ssd->parameter->chip_channel[i]* sizeof(struct chip_info));
		alloc_assert(p_channel->chip_head,"p_channel->chip_head");
		memset(p_channel->chip_head,0,ssd->parameter->chip_channel[i]* sizeof(struct chip_info));

		for (j = 0; j< ssd->parameter->chip_channel[i]; j++)
		{
			p_chip = &(p_channel->chip_head[j]);
			initialize_chip(p_chip,ssd->parameter,ssd->current_time );
		}
	}

	return ssd;
}


/*************************************************
*将page.parameters里面的参数导入到ssd->parameter里
*modify by zhouwen
*November 8,2011
**************************************************/
struct parameter_value *load_parameters(char parameter_file[30])
{
	FILE * fp;
	FILE * fp1;
	FILE * fp2;
	errno_t ferr;
	struct parameter_value *p;
	char buf[BUFSIZE];
	int i;
	int pre_eql,next_eql;
	int res_eql;
	char *ptr;

	p = (struct parameter_value *)malloc(sizeof(struct parameter_value));
	alloc_assert(p,"parameter_value");
	memset(p,0,sizeof(struct parameter_value));
	p->queue_length=REQUEST_QUEUE_LENGTH;
	memset(buf,0,BUFSIZE);
		
	if((ferr = fopen_s(&fp,parameter_file,"r"))!= 0)
	{	
		printf("the file parameter_file error!\n");	
		return p;
	}
	if((ferr = fopen_s(&fp1,"parameters_name.txt","w"))!= 0)
	{	
		printf("the file parameter_name error!\n");	
		return p;
	}
	if((ferr = fopen_s(&fp2,"parameters_value.txt","w")) != 0)
	{	
		printf("the file parameter_value error!\n");	
		return p;
	}
    


	while(fgets(buf,200,fp)){
		if(buf[0] =='#' || buf[0] == ' ') continue;
		ptr=strchr(buf,'=');
		if(!ptr) continue; 
		
		pre_eql = ptr - buf;
		next_eql = pre_eql + 1;

		while(buf[pre_eql-1] == ' ') pre_eql--;
		buf[pre_eql] = 0;
		if((res_eql=strcmp(buf,"chip number")) ==0){			
			sscanf(buf + next_eql,"%d",&p->chip_num);
		}else if((res_eql=strcmp(buf,"dram capacity")) ==0){
			sscanf(buf + next_eql,"%d",&p->dram_capacity);
		}else if((res_eql=strcmp(buf,"cpu sdram")) ==0){
			sscanf(buf + next_eql,"%d",&p->cpu_sdram);
		}else if((res_eql=strcmp(buf,"channel number")) ==0){
			sscanf(buf + next_eql,"%d",&p->channel_number); 
		}else if((res_eql=strcmp(buf,"die number")) ==0){
			sscanf(buf + next_eql,"%d",&p->die_chip); 
		}else if((res_eql=strcmp(buf,"plane number")) ==0){
			sscanf(buf + next_eql,"%d",&p->plane_die); 
		}else if((res_eql=strcmp(buf,"block number")) ==0){
			sscanf(buf + next_eql,"%d",&p->block_plane); 
		}else if((res_eql=strcmp(buf,"page number")) ==0){
			sscanf(buf + next_eql,"%d",&p->page_block); 
		}else if((res_eql=strcmp(buf,"subpage page")) ==0){
			sscanf(buf + next_eql,"%d",&p->subpage_page); 
		}else if((res_eql=strcmp(buf,"page capacity")) ==0){
			sscanf(buf + next_eql,"%d",&p->page_capacity); 
		}else if((res_eql=strcmp(buf,"subpage capacity")) ==0){
			sscanf(buf + next_eql,"%d",&p->subpage_capacity); 
		}else if((res_eql=strcmp(buf,"t_PROG")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tPROG); 
		}else if((res_eql=strcmp(buf,"t_DBSY")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tDBSY); 
		}else if((res_eql=strcmp(buf,"t_BERS")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tBERS); 
		}else if((res_eql=strcmp(buf,"t_CLS")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tCLS); 
		}else if((res_eql=strcmp(buf,"t_CLH")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tCLH); 
		}else if((res_eql=strcmp(buf,"t_CS")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tCS); 
		}else if((res_eql=strcmp(buf,"t_CH")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tCH); 
		}else if((res_eql=strcmp(buf,"t_WP")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tWP); 
		}else if((res_eql=strcmp(buf,"t_ALS")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tALS); 
		}else if((res_eql=strcmp(buf,"t_ALH")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tALH); 
		}else if((res_eql=strcmp(buf,"t_DS")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tDS); 
		}else if((res_eql=strcmp(buf,"t_DH")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tDH); 
		}else if((res_eql=strcmp(buf,"t_WC")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tWC); 
		}else if((res_eql=strcmp(buf,"t_WH")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tWH); 
		}else if((res_eql=strcmp(buf,"t_ADL")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tADL); 
		}else if((res_eql=strcmp(buf,"t_R")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tR); 
		}else if((res_eql=strcmp(buf,"t_AR")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tAR); 
		}else if((res_eql=strcmp(buf,"t_CLR")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tCLR); 
		}else if((res_eql=strcmp(buf,"t_RR")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tRR); 
		}else if((res_eql=strcmp(buf,"t_RP")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tRP); 
		}else if((res_eql=strcmp(buf,"t_WB")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tWB); 
		}else if((res_eql=strcmp(buf,"t_RC")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tRC); 
		}else if((res_eql=strcmp(buf,"t_REA")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tREA); 
		}else if((res_eql=strcmp(buf,"t_CEA")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tCEA); 
		}else if((res_eql=strcmp(buf,"t_RHZ")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tRHZ); 
		}else if((res_eql=strcmp(buf,"t_CHZ")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tCHZ); 
		}else if((res_eql=strcmp(buf,"t_RHOH")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tRHOH); 
		}else if((res_eql=strcmp(buf,"t_RLOH")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tRLOH); 
		}else if((res_eql=strcmp(buf,"t_COH")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tCOH); 
		}else if((res_eql=strcmp(buf,"t_REH")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tREH); 
		}else if((res_eql=strcmp(buf,"t_IR")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tIR); 
		}else if((res_eql=strcmp(buf,"t_RHW")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tRHW); 
		}else if((res_eql=strcmp(buf,"t_WHR")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tWHR); 
		}else if((res_eql=strcmp(buf,"t_RST")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_characteristics.tRST); 
		}else if((res_eql=strcmp(buf,"erase limit")) ==0){
			sscanf(buf + next_eql,"%d",&p->ers_limit); 
		}else if((res_eql=strcmp(buf,"flash operating current")) ==0){
			sscanf(buf + next_eql,"%lf",&p->operating_current); 
		}else if((res_eql=strcmp(buf,"flash supply voltage")) ==0){
			sscanf(buf + next_eql,"%lf",&p->supply_voltage); 
		}else if((res_eql=strcmp(buf,"dram active current")) ==0){
			sscanf(buf + next_eql,"%lf",&p->dram_active_current); 
		}else if((res_eql=strcmp(buf,"dram standby current")) ==0){
			sscanf(buf + next_eql,"%lf",&p->dram_standby_current); 
		}else if((res_eql=strcmp(buf,"dram refresh current")) ==0){
			sscanf(buf + next_eql,"%lf",&p->dram_refresh_current); 
		}else if((res_eql=strcmp(buf,"dram voltage")) ==0){
			sscanf(buf + next_eql,"%lf",&p->dram_voltage); 
		}else if((res_eql=strcmp(buf,"address mapping")) ==0){
			sscanf(buf + next_eql,"%d",&p->address_mapping); 
		}else if((res_eql=strcmp(buf,"wear leveling")) ==0){
			sscanf(buf + next_eql,"%d",&p->wear_leveling); 
		}else if((res_eql=strcmp(buf,"gc")) ==0){
			sscanf(buf + next_eql,"%d",&p->gc); 
		}else if((res_eql=strcmp(buf,"clean in background")) ==0){
			sscanf(buf + next_eql,"%d",&p->clean_in_background); 
		}else if((res_eql=strcmp(buf,"overprovide")) ==0){
			sscanf(buf + next_eql,"%f",&p->overprovide); 
		}else if((res_eql=strcmp(buf,"gc threshold")) ==0){
			sscanf(buf + next_eql,"%f",&p->gc_threshold); 
		}else if((res_eql=strcmp(buf,"buffer management")) ==0){
			sscanf(buf + next_eql,"%d",&p->buffer_management); 
		}else if((res_eql=strcmp(buf,"scheduling algorithm")) ==0){
			sscanf(buf + next_eql,"%d",&p->scheduling_algorithm); 
		}else if((res_eql=strcmp(buf,"quick table radio")) ==0){
			sscanf(buf + next_eql,"%f",&p->quick_radio); 
		}else if((res_eql=strcmp(buf,"related mapping")) ==0){
			sscanf(buf + next_eql,"%d",&p->related_mapping); 
		}else if((res_eql=strcmp(buf,"striping")) ==0){
			sscanf(buf + next_eql,"%d",&p->striping); 
		}else if((res_eql=strcmp(buf,"interleaving")) ==0){
			sscanf(buf + next_eql,"%d",&p->interleaving); 
		}else if((res_eql=strcmp(buf,"pipelining")) ==0){
			sscanf(buf + next_eql,"%d",&p->pipelining); 
		}else if((res_eql=strcmp(buf,"time_step")) ==0){
			sscanf(buf + next_eql,"%d",&p->time_step); 
		}else if((res_eql=strcmp(buf,"small large write")) ==0){
			sscanf(buf + next_eql,"%d",&p->small_large_write); 
		}else if((res_eql=strcmp(buf,"active write threshold")) ==0){
			sscanf(buf + next_eql,"%d",&p->threshold_fixed_adjust); 
		}else if((res_eql=strcmp(buf,"threshold value")) ==0){
			sscanf(buf + next_eql,"%f",&p->threshold_value); 
		}else if((res_eql=strcmp(buf,"active write")) ==0){
			sscanf(buf + next_eql,"%d",&p->active_write); 
		}else if((res_eql=strcmp(buf,"gc hard threshold")) ==0){
			sscanf(buf + next_eql,"%f",&p->gc_hard_threshold); 
		}else if((res_eql=strcmp(buf,"allocation")) ==0){
			sscanf(buf + next_eql,"%d",&p->allocation_scheme); 
		}else if((res_eql=strcmp(buf,"static_allocation")) ==0){
			sscanf(buf + next_eql,"%d",&p->static_allocation); 
		}else if((res_eql=strcmp(buf,"dynamic_allocation")) ==0){
			sscanf(buf + next_eql,"%d",&p->dynamic_allocation); 
		}else if((res_eql=strcmp(buf,"advanced command")) ==0){
			sscanf(buf + next_eql,"%d",&p->advanced_commands); 
		}else if((res_eql=strcmp(buf,"advanced command priority")) ==0){
			sscanf(buf + next_eql,"%d",&p->ad_priority); 
		}else if((res_eql=strcmp(buf,"advanced command priority2")) ==0){
			sscanf(buf + next_eql,"%d",&p->ad_priority2); 
		}else if((res_eql=strcmp(buf,"greed CB command")) ==0){
			sscanf(buf + next_eql,"%d",&p->greed_CB_ad); 
		}else if((res_eql=strcmp(buf,"greed MPW command")) ==0){
			sscanf(buf + next_eql,"%d",&p->greed_MPW_ad); 
		}else if((res_eql=strcmp(buf,"aged")) ==0){
			sscanf(buf + next_eql,"%d",&p->aged); 
		}else if((res_eql=strcmp(buf,"aged ratio")) ==0){
			sscanf(buf + next_eql,"%f",&p->aged_ratio); 
		}else if((res_eql=strcmp(buf,"queue_length")) ==0){
			sscanf(buf + next_eql,"%d",&p->queue_length); 
		}else if((res_eql=strncmp(buf,"chip number",11)) ==0)
		{
			sscanf(buf+12,"%d",&i);
			sscanf(buf + next_eql,"%d",&p->chip_channel[i]); 
		}else{
			printf("don't match\t %s\n",buf);
		}
		
		memset(buf,0,BUFSIZE);
		
	}
	fclose(fp);
	fclose(fp1);
	fclose(fp2);

	return p;
}


/*******************************
*实现request结构体的运算符=重载
********************************/
void request_assign(struct request* dest, const struct request* src) {
	if (!dest || !src) {
		return;
	}

	dest->time = src->time;
	dest->lsn = src->lsn;
	dest->size = src->size;
	dest->operation = src->operation;
	dest->complete_lsn_count = src->complete_lsn_count;
	dest->distri_flag = src->distri_flag;
	dest->begin_time = src->begin_time;
	dest->response_time = src->response_time;
	dest->energy_consumption = src->energy_consumption;

	// 释放旧的 data 避免内存泄漏
	if (dest->data) {
		free(dest->data);
		dest->data = NULL;
	}

	// 深拷贝 data
	if (src->data) {
		dest->data = (char*)malloc(strlen(src->data) + 1);
		if (dest->data) {
			strcpy(dest->data, src->data);
		}
	}

	// 浅拷贝指针
	dest->need_distr_flag = src->need_distr_flag;
	dest->subs = src->subs;
	dest->next_node = NULL;  // 这里不直接赋值 src->next_node，避免链表断裂
}


/**************
*哈希表基础操作
***************/
// 哈希函数（使用乘法散列法）
unsigned int hash_function(unsigned int ppn, int capacity) {
	const unsigned int A = 2654435761u;  // 斐波那契乘法散列数
	return (ppn * A) % capacity;
}

// 创建哈希表
HashTable* create_hash_table(int capacity) {
	HashTable* ht = (HashTable*)malloc(sizeof(HashTable));
	ht->capacity = capacity;
	ht->size = 0;
	ht->buckets = (pageData**)calloc(capacity, sizeof(pageData*));
	return ht;
}

// 插入或更新键值对
void insert(HashTable* ht, unsigned int ppn, const char* data) {
	unsigned int index = hash_function(ppn, ht->capacity);
	pageData* node = ht->buckets[index];

	// 查找是否已存在 ppn
	while (node) {
		if (node->ppn == ppn) {
			free(node->data); // 释放旧值
			node->data = (char*)malloc(strlen(data) + 1);
			if (node->data) {
				strncpy(node->data, data, strlen(data));
			}
			node->data[strlen(data)] = '\0';
			return;
		}
		node = node->next;
	}

	// 创建新节点
	pageData* new_node = (pageData*)malloc(sizeof(pageData));
	new_node->ppn = ppn;
	new_node->data = (char*)malloc(strlen(data) + 1);
	if (new_node->data) {
		strncpy(new_node->data, data, strlen(data));
	}
	new_node->next = ht->buckets[index]; // 头插法
	ht->buckets[index] = new_node;
	ht->size++;

	// 负载因子超标，扩展哈希表
	if ((float)ht->size / ht->capacity > LOAD_FACTOR) {
		rehash(ht);
	}
}

// 查找键对应的值
char* find(HashTable* ht, unsigned int ppn) {
	unsigned int index = hash_function(ppn, ht->capacity);
	pageData* node = ht->buckets[index];

	while (node) {
		if (node->ppn == ppn) {
			return node->data;
		}
		node = node->next;
	}
	return NULL; // 未找到
}

// 删除键
void erase(HashTable* ht, unsigned int ppn) {
	unsigned int index = hash_function(ppn, ht->capacity);
	pageData* node = ht->buckets[index];
	pageData* prev = NULL;

	while (node) {
		if (node->ppn == ppn) {
			if (prev) {
				prev->next = node->next;
			}
			else {
				ht->buckets[index] = node->next;
			}
			free(node->data);
			free(node);
			ht->size--;
			return;
		}
		prev = node;
		node = node->next;
	}
}

// 重新扩展哈希表
void rehash(HashTable* ht) {
	int new_capacity = ht->capacity * 2;
	pageData** new_buckets = (pageData**)calloc(new_capacity, sizeof(pageData*));

	// 重新分配所有元素
	for (int i = 0; i < ht->capacity; i++) {
		pageData* node = ht->buckets[i];
		while (node) {
			pageData* next = node->next; // 保存下一个节点
			unsigned int new_index = hash_function(node->ppn, new_capacity);

			// 头插法
			node->next = new_buckets[new_index];
			new_buckets[new_index] = node;

			node = next;
		}
	}

	free(ht->buckets);
	ht->buckets = new_buckets;
	ht->capacity = new_capacity;
}

// 打印哈希表
void print_hash_table(HashTable* ht) {
	for (int i = 0; i < ht->capacity; i++) {
		printf("Bucket %d:", i);
		pageData* node = ht->buckets[i];
		while (node) {
			printf(" [%u => %s]", node->ppn, node->data);
			node = node->next;
		}
		printf("\n");
	}
}

// 释放哈希表
void free_hash_table(HashTable* ht) {
	for (int i = 0; i < ht->capacity; i++) {
		pageData* node = ht->buckets[i];
		while (node) {
			pageData* temp = node;
			node = node->next;
			free(temp->data);
			temp->data = NULL;
			free(temp);
			temp = NULL;
		}
	}
	free(ht->buckets);
	ht->buckets = NULL;
	free(ht);
	ht = NULL;
}


