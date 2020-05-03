#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_PROCESS_NUM 10
//최대 10개의 프로세스들 까지 scheduling하는 것으로 설정
#define ALGORITHM_NUM 6
//구현한 scheduling 알고리즘은 FCFS, NON-PREEMPTIVE SJF, NON-PREEMPTIVE PRIORITY, ROUND ROBIN, PREEMPTIVE SJF, PREEMPTIVE PRIORITY 으로 총 6가지.
//추가 구현은 하지 않았습니다.

#define FCFS 0
#define SJF 1
#define PRIORITY 2
#define RR 3

#define TRUE 1
#define FALSE 0
#define TIME_QUANTUM 3
//본 simulator에서 사용할 ROUND ROBIN 알고리즘에서의 타임 퀀텀은 3으로 설정하였다.

typedef struct Process* processPointer;
typedef struct Process {
    int pid;
    int priority;
    int arrivalTime;
    int CPUburst;
    int IOburst;
    int CPUremainingTime;
    int IOremainingTime;
    int waitingTime;
    int turnaroundTime;
    int responseTime;
}Process;
//프로세스 구조체
int running_label[100];
//ganttchart display를 위한 배열, 현 time_count 시점에서의 runningProcess의 pid를 저장할 것.

typedef struct evaluation* evalPointer;
typedef struct evaluation {
	int algorithm;
	int preemptive;
	//scheduling 알고리즘 명세
	int avg_waitingTime;
	int avg_turnaroundTime;
	int avg_responseTime;
	int completedProcess;

}evaluation;
//사용할 scheduling algorithm별 평가값들을 저장하는 구조체

evalPointer evals[ALGORITHM_NUM];
int evaluation_num = 0;

void initialize_evals(){
	evaluation_num = 0;
	int i;
	for(i=0;i<ALGORITHM_NUM;i++)
		evals[i]=NULL;
}
//평가값 구조체 배열 초기화

void clear_evals() {

	int i;
	for(i=0;i<ALGORITHM_NUM;i++){
		free(evals[i]);
		evals[i]=NULL;
	}
	evaluation_num = 0;
}
//시뮬레이터 실행 후 메모리 반환, 초기화

//Job Queue
//simulator 작동과 동시에 프로세스들이 생성(createProcesses())되고, 생성된 프로세스들을 load받아서 잡큐에 넣는다.
processPointer jobQueue[MAX_PROCESS_NUM];
int process_num_JQ = 0;

void initialize_JQ () {
	process_num_JQ = 0;
    int i;
    for (i = 0; i < MAX_PROCESS_NUM; i++)
        jobQueue[i] = NULL;
}

int search_JQ (int searchPid) { //readyQueue에서 해당 pid를 가지고 있는 process의 index를 리턴한다.
    int result = -1;
    int i;
    for(i = 0; i < process_num_JQ; i++) {
        int temp = jobQueue[i]->pid;
        //포인터로 해당 pid를 가리키고 있음
        if(temp == searchPid)
            return i;
            //인덱스 i가 리턴되면 해당 process가 존재함
    }
    return result;
    //-1이 리턴되면 해당 pid를 가진 process가 없는 것
}

void insert_JQ (processPointer proc) {
    if(process_num_JQ<MAX_PROCESS_NUM) {
        int temp = search_JQ(proc->pid);
        if (temp != -1) {
        //-1이 아니라 process의 index가 리턴되면 이미 큐에 해당 pid의 process가 존재하는 것
            //printf("중복되는 pid의 프로세스가 이미 존재합니다.", proc->pid);
            return;
        }
        jobQueue[process_num_JQ++] = proc;
        //큐의 맨 뒤에 넣음
    }
    else {
        puts("최대 10개 까지의 프로세스들을 load할 수 있습니다.");
        //최대 프로세스 수에 도달하면 더이상 넣을 수 없으므로 에러 발생
        return;
    }
}

processPointer removeFrom_JQ (processPointer proc) { //process 하나를 readyQueue에서 제거하고 빈 공간을 수축을 통해 없앤다.
    if(process_num_JQ>0) {
        int temp = search_JQ(proc->pid);
        if (temp == -1) {
        //-1이면 해당 pid를 가진 프로세스가 없음. 에러 발생
            //printf("해당 프로세스가 존재하지 않습니다.", proc->pid);
            return NULL;
        } else {
            processPointer removed = jobQueue[temp];
            //받은 index의 위치에 있는 프로세스를 removed 포인터에 저장

            int i;
            for(i = temp; i < process_num_JQ - 1; i++)
                jobQueue[i] = jobQueue[i+1];
                //선택한 프로세스를 빼내고 그 뒤에 있던 프로세스들을 한칸씩 앞으로 당김
            jobQueue[process_num_JQ - 1] = NULL;
            //프로세스 하나가 없어졌으므로, 맨뒤 공간을 null로 처리

            process_num_JQ--;
            //프로세스 하나가 없어졌으므로, 현 프로세스 수 감소
            return removed;
            //빼낸 프로세스 리턴
        }

    } else {
        puts("더이상 프로세스가 존재하지 않습니다.");
        //프로세스 수 = 0
        return NULL;
    }
}

void clear_JQ() { //메모리 회수용 함수, 메모리 공간을 비워주고, 큐를 초기화 시킨다.
    int i;
    for(i = 0; i < MAX_PROCESS_NUM; i++) {
        free(jobQueue[i]);
        jobQueue[i] = NULL;
    }
    process_num_JQ = 0;
}

void print_JQ() {
//생성되어 잡큐로 load된 프로세스들의 속성들을 display
	printf("프로세스 : %d개\n", process_num_JQ);
	int i;
	puts("pid    priority    arrival_time    CPU burst    IO burst");
    for(i = 0; i < process_num_JQ; i++) {
    puts ("-----------------------------------------------------------");
        printf("%3d    %8d    %12d    %9d    %8d\n", jobQueue[i]->pid, jobQueue[i]->priority, jobQueue[i]->arrivalTime, jobQueue[i]->CPUburst, jobQueue[i]->IOburst);
    }
}

processPointer copyJobQueue[MAX_PROCESS_NUM];
int process_num_copy_JQ = 0;

void Copy_JQ() {
	// 여러 시뮬레이션을 처리하기 위해 copy을 만들어준다.

	int i;
	for (i=0; i< MAX_PROCESS_NUM; i++) {
		copyJobQueue[i] = NULL;
	}
	//복제 잡큐 초기화

	for (i=0; i<process_num_JQ; i++) {

		processPointer newProcess = (processPointer)malloc(sizeof(struct Process));
        newProcess->pid = jobQueue[i]->pid;
        newProcess->priority = jobQueue[i]->priority;
        newProcess->arrivalTime = jobQueue[i]->arrivalTime;
        newProcess->CPUburst = jobQueue[i]->CPUburst;
        newProcess->IOburst = jobQueue[i]->IOburst;
        newProcess->CPUremainingTime = jobQueue[i]->CPUremainingTime;
        newProcess->IOremainingTime = jobQueue[i]->IOremainingTime;
        newProcess->waitingTime = jobQueue[i]->waitingTime;
        newProcess->turnaroundTime = jobQueue[i]->turnaroundTime;
        newProcess->responseTime = jobQueue[i]->responseTime;
        //현재 잡큐에 존재하는 프로세스들의 정보를 그대로 다 카피해서 복제 잡큐를 만들어준다.
        //여러 시뮬레이션들을 동일한 프로세스 셋으로 실행하기 위함.

        copyJobQueue[i] = newProcess;
	}

	process_num_copy_JQ = process_num_JQ;
	//복제 잡큐는 실제 잡큐와 동일한 정보를 가진다.
}

void loadCopy_JQ() {
	// 복제 잡큐의 데이터를 실제 잡큐공간에 복사한다.
	clear_JQ();
	//먼저 잡큐를 비워주고
	int i;
	for (i=0; i<process_num_copy_JQ; i++) {

		processPointer newProcess = (processPointer)malloc(sizeof(struct Process));
	    newProcess->pid = copyJobQueue[i]->pid;
	    newProcess->priority = copyJobQueue[i]->priority;
	    newProcess->arrivalTime = copyJobQueue[i]->arrivalTime;
	    newProcess->CPUburst = copyJobQueue[i]->CPUburst;
	    newProcess->IOburst = copyJobQueue[i]->IOburst;
	    newProcess->CPUremainingTime = copyJobQueue[i]->CPUremainingTime;
	    newProcess->IOremainingTime = copyJobQueue[i]->IOremainingTime;
	    newProcess->waitingTime = copyJobQueue[i]->waitingTime;
	    newProcess->turnaroundTime = copyJobQueue[i]->turnaroundTime;
	    newProcess->responseTime = copyJobQueue[i]->responseTime;
	    //실제 시뮬레이션을 구동하기 위해 만들어놓은 복제 잡큐들을 실제 잡큐들로 가정하고 실행하기 위해 복사한것을 그대로 옮겨준다.

	    jobQueue[i] = newProcess;
	}
	process_num_JQ = process_num_copy_JQ;
}

void clearCopy_JQ() { //메모리 회수용 함수, 메모리를 비워주고, 초기화 시킴
    int i;
    for(i = 0; i < MAX_PROCESS_NUM; i++) {
        free(copyJobQueue[i]);
        copyJobQueue[i] = NULL;
    }
}
//running state 현재 funning 중인 process, 초기값은 null이고, 소비시간은 0
processPointer runningProcess = NULL;
int elapsedTime = 0;
//Round Robin 기법에서 프로세스의 턴에서의 elapsed time을 측정하기 위한 변수

//readyQueue
//도착시간에 따라 순서대로 정렬된 채로 프로세스가 생성된다고 가정. 시뮬레이션이 실행될때, 프로세스들은 도착시간이 되었을 때,
//job scheduler를 통해 레디큐로 schedule되어 들어가는 구조로 구현
processPointer readyQueue[MAX_PROCESS_NUM];
int process_num_RQ = 0; // 현재 process의 수

void initialize_RQ () {
    process_num_RQ = 0;
	int i;
    for (i = 0; i < MAX_PROCESS_NUM; i++)
        readyQueue[i] = NULL;
}

int search_RQ (int searchPid) { //레디큐에서 해당 pid를 가지고 있는 프로세스의 index를 리턴한다.
    int result = -1;
    int i;
    for(i = 0; i < process_num_RQ; i++) {
        int temp = readyQueue[i]->pid;
        if(temp == searchPid)
            return i;
    }
    return result;
}

void insert_RQ (processPointer proc) {
    if(process_num_RQ<MAX_PROCESS_NUM) {
        int temp = search_RQ(proc->pid);
        if (temp != -1) {
            //printf("해당 프로세스가 이미 존재합니다.\n", proc->pid);
            return;
        }
        readyQueue[process_num_RQ++] = proc;
        //해당 pid 프로세스가 없으면 큐의 맨뒤에 넣어줌
    }
    else {
        puts("최대 job schedule 할 수 있는 프로세스 수가 초과 되었습니다.");
        return;
    }
}

processPointer removeFrom_RQ (processPointer proc) { //process 하나를 readyQueue에서 제거하고 빈 공간을 수축을 통해 없앤다.
    if(process_num_RQ>0) {
        int temp = search_RQ(proc->pid);
        if (temp == -1) {
            //printf("해당 프로세스가 존재하지 않습니다.\n", proc->pid);
            return NULL;
        } else {
            processPointer removed = readyQueue[temp];

            int i;
            for(i = temp; i < process_num_RQ - 1; i++)
                readyQueue[i] = readyQueue[i+1];
            readyQueue[process_num_RQ - 1] = NULL;

            process_num_RQ--;
            return removed;
        }

    } else {
        puts("더 이상 scheduling할 프로세스가 없습니다.");
        return NULL;
    }
}

void clear_RQ() { //메모리 회수용 함수
    int i;
    for(i = 0; i < MAX_PROCESS_NUM; i++) {
        free(readyQueue[i]);
        readyQueue[i]=NULL;
    }
    process_num_RQ = 0;
}

void print_RQ() { //debug를 위한 print 함수
    puts("\nprintf_RQ()");
	int i;
    for(i = 0; i < process_num_RQ; i++) {
        printf("%d ", readyQueue[i]->pid);
    }
    printf("\n프로세스 : %d개\n", process_num_RQ);
}

//waitingQueue
processPointer waitingQueue[MAX_PROCESS_NUM];
int process_num_WQ = 0;

void initialize_WQ () {
	process_num_WQ = 0;
    int i;
    for (i = 0; i < MAX_PROCESS_NUM; i++)
        waitingQueue[i] = NULL;
}

int search_WQ (int searchPid) { //대기큐에서 해당 pid를 가지고 있는 프로세스의 index를 리턴한다.
    int result = -1;
    int i;
    for(i = 0; i < process_num_WQ; i++) {
        int temp = waitingQueue[i]->pid;
        if(temp == searchPid)
            return i;
    }
    return result;
}

void insert_WQ (processPointer proc) {
    if(process_num_WQ<MAX_PROCESS_NUM) {
        int temp = search_WQ(proc->pid);
        if (temp != -1) {
            //printf("해당 프로세스가 이미 IO 작업을 수행중입니다.\n", proc->pid);
            return;
        }
        waitingQueue[process_num_WQ++] = proc;
    }
    else {
        puts("현재 더 이상 IO 작업을 수행할 수 없습니다.");
        return;
    }
    //print_WQ();
}

processPointer removeFrom_WQ (processPointer proc) { //프로세스 하나를 대기큐에서 제거하고 빈 공간을 수축을 통해 없앤다.
    if(process_num_WQ>0) {
        int temp = search_WQ(proc->pid);
        if (temp == -1) {
            //printf("해당 프로세스가 없습니다.", proc->pid);
            return NULL;
        } else {

            processPointer removed = waitingQueue[temp];
            int i;
            for(i = temp; i < process_num_WQ - 1; i++)
                waitingQueue[i] = waitingQueue[i+1];

            waitingQueue[process_num_WQ - 1] = NULL;

            process_num_WQ--;

            return removed;
        }
    } else {
        puts("현재 IO 작업을 수행중인 프로세스가 없습니다.");
        return NULL;
    }
}

void clear_WQ() { //메모리 회수용 함수
    int i;
    for(i = 0; i < MAX_PROCESS_NUM; i++) {
        free(waitingQueue[i]);
        waitingQueue[i] = NULL;
    }
    process_num_WQ = 0;
}

void print_WQ() {
    puts("\nprintf_WQ()");
	int i;

    for(i = 0; i < process_num_WQ; i++) {
        printf("%d ", waitingQueue[i]->pid);
    }
    printf("\n프로세스 : %d개\n", process_num_WQ);
}

//terminatedQueue
processPointer terminated[MAX_PROCESS_NUM];
int process_num_TQ = 0;

void initialize_TQ () {
	process_num_TQ = 0;
    int i;
    for (i = 0; i < MAX_PROCESS_NUM; i++)
        terminated[i] = NULL;
}

void clear_TQ() { //메모리 회수용 함수
    int i;
    for(i = 0; i < MAX_PROCESS_NUM; i++) {
        free(terminated[i]);
        terminated[i] = NULL;
    }
    process_num_TQ = 0;
}

void insert_TQ (processPointer proc) {
    if(process_num_TQ<MAX_PROCESS_NUM) {
        terminated[process_num_TQ++] = proc;
    }
    else {
        puts("현재 프로세스를 종료할 수 없습니다.");
        return;
    }
}

void print_TQ() { //debug를 위한 print 함수
    puts("\nprintf_TQ()");

	int i;
    for(i = 0; i < process_num_TQ; i++) {
        printf("%d ", terminated[i]->pid);
    }
    printf("\n프로세스 : %d개\n", process_num_TQ);
}

processPointer createProcess(int pid, int priority, int arrivalTime, int CPUburst, int IOburst) {
//프로세스 하나를 만든다.
        processPointer newProcess = (processPointer)malloc(sizeof(struct Process));
        newProcess->pid = pid;
        newProcess->priority = priority;
        newProcess->arrivalTime = arrivalTime;
        newProcess->CPUburst = CPUburst;
        newProcess->IOburst = IOburst;
        newProcess->CPUremainingTime = CPUburst;
        newProcess->IOremainingTime = IOburst;
        //remaining time들의 초기값은 부여받은 총 burst time과 같다.
        newProcess->waitingTime = 0;
        newProcess->turnaroundTime = 0;
        newProcess->responseTime = -1;
        //잡큐에 넣는다.
        insert_JQ(newProcess);
        //프로세스가 처음 생성되면 잡큐로 간다.

    return newProcess;
}

processPointer FCFS_algorithm() {
        processPointer candidate = readyQueue[0]; //가장 먼저 도착한 process를 찾는다.
        //잡큐에서 도착시간 순서에 따라 레디큐로 넘어감(도착시간이 같다면, 생성된 순(pid순))

        if (candidate != NULL){

            if(runningProcess != NULL) { //이미 수행중인 프로세스가 있었다면 preemptive가 아니므로 기다린다.
            //이전 RunSystem count(time_count)에서 아직 terminate되거나 IO reauest가 발생하지 않았음
				return runningProcess;
				//scheduling 기법을 통해 해당 카운트 시점(time_count)에 선별된, cpu자원을 할당받을, 수행될 프로세스를 반환하는 것
        	} else {
				return removeFrom_RQ(candidate);
				//FCFS 기법에 따라 가장 먼저 도착한 순서로 프로세스 수행
			}
        } else { //레디큐에 아무것도 없는 경우
            return runningProcess;
            //더이상 schedule할 프로세스가 없으므로, 그대로 마저 수행
        }
}

processPointer SJF_algorithm(int preemptive) {

	processPointer candidate = readyQueue[0];
	//일단은 레디큐의 맨 앞 프로세스로 초기화

	if(candidate != NULL) {
	//candidate을 찾는 과정
		int i;
        for(i = 0; i < process_num_RQ; i++) {
            if (readyQueue[i]->CPUremainingTime <= candidate->CPUremainingTime) {
            //레디큐 내의 프로세스들의 남은 cputime을 비교한다.

                if(readyQueue[i]->CPUremainingTime == candidate->CPUremainingTime) { //남은 시간이 같을 경우먼저 도착한 process가 먼저 수행된다.
                    if (readyQueue[i]->arrivalTime < candidate->arrivalTime) candidate = readyQueue[i];
                    //도착시간까지 같다면 굳이 candidate을 update하지는 않는다.
                } else {
                    candidate = readyQueue[i];
                    //남은 cputime이 적은 것 부터 실행하므로, candidate update
                }
            }
        }

		if(runningProcess != NULL) { //이미 수행중인 프로세스가 있을 때, preept과 관련된 코드임.
				if(preemptive){ //preemptive면

					if(runningProcess->CPUremainingTime >= candidate->CPUremainingTime) {
					//실행중인 프로세스의 남은 cputime과 현 candidate의 cputime을 비교
						if(runningProcess->CPUremainingTime == candidate->CPUremainingTime) { //남은 시간이 같을 경우먼저 도착한 process가 먼저 수행된다.
		                    if (runningProcess->arrivalTime < candidate->arrivalTime){
								return runningProcess;
							} else if(runningProcess->arrivalTime == candidate->arrivalTime)
								return runningProcess; //도착시간까지 같으면 굳이 preempt 발생시키지 않는다. 마저 수행한다.
						}

						//남은 시간이 candidate이 더 짧다면,
						insert_RQ(runningProcess);
						//실행중이던 프로세스가 preempt되면서 다시 레디큐의 맨뒤로 들어감
						return removeFrom_RQ(candidate);
						//Context switch 발생
						//수행하기 위해 레디큐에서 빼냄.
					}

					return runningProcess;
					//현 수행중인 프로세스가 위의 preempt조건에 걸리는 상황이 생기지 않으면 그대로 계속 수행한다.
				}
				return runningProcess;
				//non-preemptive면 기다린다. 역시 현 수행중인 프로세스 그대로 계속 수행한다.
        	} else {
				return removeFrom_RQ(candidate);
				//이전 RunSystem count(time_count)에서 프로세스가 terminate되거나 IO request가 발생하여 수행중인 프로세스가 없다면
			}

	}else {
		return runningProcess;
		//레디큐에 남은 프로세스가 없음, scheduling없이 그대로 마저 수행한다.
	}
}

processPointer PRIORITY_algorithm(int preemptive) {
	processPointer candidate = readyQueue[0];
	//priority의 크기가 작을 수록 우선순위는 높다.
//레디큐의 첫번째 프로세스로 초기화
	if(candidate != NULL) {
		int i;
        for(i = 0; i < process_num_RQ; i++) {
            if (readyQueue[i]->priority <= candidate->priority) {
                //프로세스들의 우선순위를 비교
                if(readyQueue[i]->priority == candidate->priority) { //priority가 같을 경우먼저 도착한 process가 먼저 수행된다.
                    if (readyQueue[i]->arrivalTime < candidate->arrivalTime)
						candidate = readyQueue[i];
						//도착시간까지 같다면 굳이 candidate을 update하지 않음
                } else {
                    candidate = readyQueue[i];
                    //현 레디큐에 존재하는 다음 프로세스의 우선순위가 더 높음
                }
            }
        }

		if(runningProcess != NULL) { //이미 수행중인 프로세스가 있을 때
				if(preemptive){ //preemptive면
					if(runningProcess->priority >= candidate->priority) {
						if(runningProcess->priority == candidate->priority) { //priority가 같을 경우먼저 도착한 process가 먼저 수행된다.
		                    if (runningProcess->arrivalTime < candidate->arrivalTime){
								return runningProcess;// 수행중인 프로세스가 더 빨리 도착함.
							} else if(runningProcess->arrivalTime == candidate->arrivalTime) {
								return runningProcess; //arrivalTime까지 같다면 굳이 preempt안한다.
							}
						}
						insert_RQ(runningProcess);
						//현 candidate의 우선순위가 더 높으므로(priority값이 더 작거나, 만약 같을때, 더 빨리 도착했다면,)
						//수행중인 프로세스를 다시 레디큐의 맨뒤로 넣고
						return removeFrom_RQ(candidate);
						//현 candidate을 레디큐에서 빼낸다. Context switch발생
					}

					return runningProcess;
					//위의 preempt 조건에 걸리지 않으면 preempt되지 않고 계속 수행함
				}

				return runningProcess;
				//non-preemptive면 그대로 계속 수행
        	} else {
				return removeFrom_RQ(candidate);
				//이미 수행중인 프로세스가 없음 이전에 수행되던 프로세스가 terminate됬거나, IO request발생-> 현 candidate을 레디큐에서 빼냄.
			}

	}else {
		return runningProcess;
		//레디큐가 비어있음. 다음 프로세스가 없으므로, scheduling 없이 현재 수행중인 프로세스 그대로 진행
	}
}

processPointer RR_algorithm(int time_quantum){
	processPointer candidate = readyQueue[0]; //가장 먼저 도착한 프로세스를 찾는다.
	//Round Robin의 기본적인 프로세스 scheduling의 순서는 FCFS와 같으므로, 알고리즘내에서 candidate의 별도의 update과정이 없다
        if (candidate != NULL){
            if(runningProcess != NULL) { //이미 수행중인 프로세스가 있었다면
				if(elapsedTime >= TIME_QUANTUM){ //이미 수행중이 었던 프로세스가 타임 퀀텀을 모두 소비하였다면,
					insert_RQ(runningProcess);
					//다시 레디큐의 맨뒤로 넣어준다.
					return removeFrom_RQ(candidate);
					//다음 프로세스를 레디큐에서 빼냄.
				} else {
					return runningProcess;
					//아직 타임퀀텀이 남아있으므로 마저 그대로 수행
				}

        	} else {
				return removeFrom_RQ(candidate);
				//수행중인 프로세스가 없으므로 레디큐에서 가장 먼저 도착한 프로세스를 빼냄.
				//Round Robin의 기본적인 프로세스 scheduling의 순서는 FCFS와 같으므로
			}

        } else { //readyQueue에 아무것도 없는 경우
            return runningProcess;
            //레디큐가 비어있으므로, 다음 scheduling할 프로세스가 없음. 현재 수행중인 프로세스를 마저 그대로 수행
        }
}



processPointer schedule(int algorithm, int preemptive, int time_quantum) {
	processPointer scheduled = NULL;
	//각 scheduling 기법을 사용하였을 때, 각 RunSystem count(time_count)에서 CPU자원을 할당받아 수행될 프로세스를 선택하여 반환한다.

    switch(algorithm) {
        case FCFS:
            scheduled = FCFS_algorithm();
            break;
        case SJF:
        	scheduled = SJF_algorithm(preemptive);
        	break;
        case RR:
        	scheduled = RR_algorithm(time_quantum);
        	break;
        case PRIORITY:
        	scheduled = PRIORITY_algorithm(preemptive);
        	break;
        default:
        return NULL;
    }
    return scheduled;
}

void RunSystem(int time_count, int algorithm, int preemptive, int time_quantum) {
//simulation에서 time_count 시간을 증가시키면서 반복문으로 계속 실행시킬 것임->time_count번 실행, 매 순간마다의 상태를 체크
	//우선, Job queue에서 해당 시간에 도착한 프로세스들을 ready queue에 올려준다.(job scheduling)
	processPointer tempProcess = NULL;
	int i;
	for(i = 0; i < process_num_JQ; i++) {
		if(jobQueue[i]->arrivalTime == time_count) {
		//잡큐에서 도착시간이 현재 time_count시간과 같다면, 프로세스가 레디큐에 도착해야한다,
		//**만약 arrival time이 같은 프로세스들이 존재한다면, 먼저 생성되어 먼저 잡큐로 들어간, 프로세스가 레디큐로 먼저 들어간다.(pid의 오름차순)**
			tempProcess = removeFrom_JQ(jobQueue[i--]);//remove를 하면 큐가 수축하므로, i 인덱스를 감소시켜줘야, 다음
			//인덱스가 증가하였을때, 다음 프로세스를 서치할 수 있음
			insert_RQ(tempProcess);
		}
	}
	processPointer oldProcess = runningProcess;
	//이미 수행된 프로세스를 이전 time_count 프로세스로 저장하고,
	runningProcess = schedule(algorithm, preemptive, time_quantum);
	//이번 turn에 수행될 프로세스를 scheduling을 통해 선별한다.
	if(oldProcess != runningProcess) { //이전과 다른 프로세스가 running 상태로 되었을 경우
		elapsedTime = 0;
		//running에 소요된 시간을 초기화시켜준다. -> 새로운 프로세스가 수행되므로
		if(runningProcess->responseTime == -1) { //responseTime을 기록해둔다.
			runningProcess->responseTime = time_count - runningProcess->arrivalTime;
			//responsetime의 초기값은 -1, responsetime은 도착하고, 첫 리퀘스트가 발생할때까지의 시간이므로, 도착(arrival time)
			//부터 현재 이 프로세스가 scheduling을 통해 선별되어 첫 수행되는 time_count시점까지의 시간이므로 time_count - arrivaltime 이 responsetime
		}
	}

    for(i = 0; i < process_num_RQ; i++) {
        if(readyQueue[i]) {
        //프로세스들이 레디큐에 있다면, waitingtime과 turnaroundtime이 증가
        	readyQueue[i]->waitingTime++;
        	readyQueue[i]->turnaroundTime++;
    	}
    }//1time_count가 지나는 순간이므로 1씩 증가해줌

    for(i = 0; i < process_num_WQ; i++) { //대기큐에 있는 프로세스들이 IO 작업을 수행한다.
		if(waitingQueue[i]) {
			//IO작업을 수행중이므로, waitingtime을 증가시키지는 않음.
			waitingQueue[i]->turnaroundTime++;
			waitingQueue[i]->IOremainingTime--;
			//IO대기큐에 존재 하는 거 자체로 IO 작업을 수행한다고 가정
			//1time_count가 지나는 순간이므로 1씩 감소해줌

			if(waitingQueue[i]->IOremainingTime <= 0 ) { //IO 작업이 완료된 경우
			//한번 IO request가 발생하여 대기큐에 들어오면, IO 작업이 다 완료되어야 다시 레디큐로 돌아갈 수 있다.
				insert_RQ(removeFrom_WQ(waitingQueue[i--])); //ready queue로 프로세스를 다시 돌려보내준다.
				//print_WQ();
				//대기큐가 수축하므로 서치 인덱스 감소시켜줌.
			}
		}
	}

    if(runningProcess != NULL) {
    //수행 중인 프로세스가 있다면 실행시킴
        runningProcess->CPUremainingTime --;
        //수행중이므로 cpuremainingtime 감소
        runningProcess->turnaroundTime ++;
        //수행중이므로, turnaroundtime만 증가
        elapsedTime ++;
        //소요시간도 당연히 증가
        running_label[time_count] = runningProcess->pid;//현 time_count시점에서 수행중인 프로세스의 pid를 저장한다.


        if(runningProcess->CPUremainingTime <= 0 && runningProcess->IOremainingTime <= 0) {
    //CPUburst, IO burst 모두 수행이 된 상태라면, terminated로 보내준다.
			insert_TQ(runningProcess);
			runningProcess = NULL;
			//수행이 완료되었으므로, 초기화
		}
        else if (runningProcess->CPUremainingTime > 0 && runningProcess->IOremainingTime > 0 ){
		//CPUburst, IO burst 모두 수행이 덜 된 상태라면, 1/3의 확률로 IO burst를 발생시킨다.
		//랜덤 IO 발생
		      srand(time_count);
              if((rand() % 3) == 1){
				insert_WQ(runningProcess);
				runningProcess = NULL;
				//IO작업을 위해 대기큐에 가면, 다시 수행중인 프로세스를 초기화 해준다.
              }
			}
            else if(runningProcess->CPUremainingTime <= 0 && runningProcess->IOremainingTime > 0){
                //CPU burst는 모두 수행했는데 IO burst만 남아 있다면, 정해진 simulation 카운트 내에 프로세스를 완료하기 위해서
                //100퍼센트 확률로 IO burst를 발생시킨다.
                //이 부분은 고민을 많이 했습니다. 1/3확률로 계속 랜덤하게 발생시키다가는 정해진 시간내에 프로세스를 종료시키지
                //못할수도 있을거 같아서 제 나름대로의 policy를 만들었습니다.
                insert_WQ(runningProcess);
				runningProcess = NULL;
				//IO작업을 위해 대기큐에 가면, 다시 수행중인 프로세스를 초기화 해준다.
            }
    }
}
void Partial_Evaluate(int algorithm, int preemptive) {
//알고리즘 평가값들을 산출하여 평가 구조체에 저장

	int wait_sum = 0;
	int turnaround_sum = 0;
	int response_sum = 0;
	int i;
	processPointer p=NULL;

    for(i=0;i<process_num_TQ;i++){
		p = terminated[i];
//		//terminate된 프로세스들의 time값들을 받는다.
		wait_sum += p->waitingTime;
		turnaround_sum += p->turnaroundTime;
		response_sum += p->responseTime;
		//terminate된 프로세스들의 time값의 총합을 구함.
	}

	if(process_num_TQ != 0) {
	//평가값들을 저장
		evalPointer newEval = (evalPointer)malloc(sizeof(struct evaluation));
		newEval->algorithm = algorithm;
		newEval->preemptive = preemptive;
		//평가할 알고리즘 명세
		newEval->avg_waitingTime = wait_sum/process_num_TQ;
		newEval->avg_turnaroundTime = turnaround_sum/process_num_TQ;
		newEval->avg_responseTime = response_sum/process_num_TQ;
		newEval->completedProcess = process_num_TQ;
		evals[evaluation_num++] = newEval;
	}
}
void simulation(int algorithm, int preemptive, int time_quantum, int count) {
	loadCopy_JQ();
	//시뮬레이션 구동을 위해 카피해둔 잡큐를 불러온다.
	switch(algorithm) {
        case FCFS:
            puts("FCFS");
            break;
        case SJF:
        	if(preemptive) printf("Preemptive ");
        	else printf("Non-preemptive ");
        	puts("SJF");
        	break;
        case RR:
        	printf("Round Robin\n");
        	break;
        case PRIORITY:
        	if(preemptive) printf("Preemptive ");
        	else printf("Non-preemptive ");
        	puts("Priority");
        	break;
        default:
        return;
    }

	int initial_process_num = process_num_JQ; //실제 시뮬레이션을 하기 전 프로세스의 수를 저장해둔다.
	int i;
	if(process_num_JQ <= 0) {
	//프로세스가 없다면 구동 불가.
		puts("-에러- 프로세스가 없습니다.");
		return;
	}

	for(i=0;i<count;i++) {
	//time_count, 즉 시간 시점을 계속 증가시키면서 RunSystem 실행 입력한 time_count값까지 실행(120)
		//RunSystem와 동시에 수행중인 프로세스 pid를 GanttChart형식으로 display
		//P-1은 idle을 의미함
		running_label[i] = -1;
		//RunSystem 전에는 idle로 초기화
		RunSystem(i,algorithm, preemptive, TIME_QUANTUM);
		if(process_num_TQ != initial_process_num) {
		        if(i == 0){
		    printf("P%d-", running_label[i]);
		    }
		    else{
		      if(running_label[i-1] == running_label[i]){
		         printf("-");
		         }
		      else{
		           printf("lP%d-", running_label[i]);
		      }
		    }
		}
		else {
		//처음 설정해준 프로세스가 모두 terminate되면, counting종료
			printf("l");
			break;
		}
	}
    printf("\n");
    printf("\n");
	Partial_Evaluate(algorithm, preemptive);
	//알고리즘별 평가값 저장
	clear_JQ();
    clear_RQ();
    clear_TQ();
    clear_WQ();
    //다음 알고리즘 수행을 위해
    //큐 메모리 반환, 초기화
    free(runningProcess);
    runningProcess = NULL;
    //running프로세스 초기화
    elapsedTime = 0;
}

void Total_Evaluate() {
//총평 부분 Partial_Evaluate에서 저장해놓은 평가구조체의 값들을 출력함
puts ("-----------------------------------------------------------");
	int i;
	for(i=0;i<evaluation_num;i++) {
		int algorithm = evals[i]->algorithm;
		int preemptive = evals[i]->preemptive;
puts ("-----------------------------------------------------------");
		switch (evals[i]->algorithm) {

		case FCFS:
            puts("FCFS");
            break;
        case SJF:
        	if(preemptive) printf("Preemptive ");
        	else printf("Non-preemptive ");
        	puts("SJF");
        	break;
        case RR:
        	puts("Round Robin");
        	break;
        case PRIORITY:
        	if(preemptive) printf("Preemptive ");
        	else printf("Non-preemptive ");
        	puts("Priority");
        	break;
        default:
        return;
		}
		printf("completedProcess: %d\n",evals[i]->completedProcess);
		printf("Average waiting time: %d\n",evals[i]->avg_waitingTime);
		printf("Average turnaround time: %d\n",evals[i]->avg_turnaroundTime);
		printf("Average response time: %d\n",evals[i]->avg_responseTime);
	}
}


void createProcesses(int total_num) {
//total_num : 전체 프로세스 수(코드 실행시 입력)
	srand(time(NULL));

	int i;
	for(i=0;i<total_num; i++) {
		createProcess(i+1, rand() % total_num + 1, rand() % (total_num + 5), rand() % total_num + 5, rand() % (total_num + 10));
		//createProcess(int pid, int priority, int arrivalTime, int CPUburst, int IOburst)

		//프로세스는 pid가 오름차순으로 생성되므로 잡큐에서 자동으로 pid순으로 정렬된다.
		//그러므로 RunSystem에서 잡큐에서 레디큐로 프로세스를 올려줄 때, arrival time이 같은 프로세스들 중에서는 pid가 작은,
		//즉 먼저 생성되어 잡큐에 먼저 들어온 프로세스가 먼저 레디큐로 올라간다.
	}
	Copy_JQ();
	//다수의 sheduling기법을 대상으로 simulator 구동을 위해 카피본을 생성
	print_JQ();
	//코드 실행시 생성된 프로세스 속성을 display
}

void main(int argc, char **argv) {
    initialize_RQ();
    initialize_JQ();
    initialize_TQ();
    initialize_WQ();
    initialize_evals();

    int totalProcessNum = atoi(argv[1]);
    createProcesses(totalProcessNum);
    //프로세스 set 생성
    int i;
    int time_count = 100;
    //100번까지 카운트하며, 각 카운트 시점마다 scheduling이 발생한다.
 	simulation(FCFS,FALSE,TIME_QUANTUM, time_count);
    simulation(SJF,FALSE,TIME_QUANTUM, time_count);
    simulation(SJF,TRUE,TIME_QUANTUM, time_count);
	simulation(PRIORITY,FALSE,TIME_QUANTUM, time_count);
	simulation(PRIORITY,TRUE,TIME_QUANTUM, time_count);
	simulation(RR,TRUE,TIME_QUANTUM, time_count);
	Total_Evaluate();

	clear_JQ();
    clear_RQ();
    clear_TQ();
    clear_WQ();
    clearCopy_JQ();
	clear_evals();
}