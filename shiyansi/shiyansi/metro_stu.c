/**
 * 地铁线路图查询器（学生版）
 * 实验任务：完成所有标记为 // TODO 的函数实现。
 * 编译：gcc -o metro metro_student.c -std=c99
 * 运行：./metro
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MAX_NAME_LEN 32
#define MAX_LINE_NAME 20

// 邻接表边结点
typedef struct EdgeNode {
    int adjVex;               // 邻接站点编号
    int weight;               // 权值（运行时间，分钟）
    struct EdgeNode *next;
} EdgeNode;

// 顶点结点（站点）
typedef struct VertexNode {
    char name[MAX_NAME_LEN];  // 站点名称
    EdgeNode *firstEdge;      // 第一条边
    int *lineIds;             // 该站点所属的线路编号数组（动态分配）
    int lineCount;            // 所属线路数量
} VertexNode;

// 图结构
typedef struct {
    VertexNode *vertices;     // 顶点数组
    int vertexNum;            // 实际顶点数
    int vertexCapacity;       // 顶点数组容量
    int edgeNum;              // 边数
    int isDirected;           // 0:无向, 1:有向
} Graph;

// 辅助队列（用于BFS）
typedef struct Queue {
    int *data;
    int front, rear, size, capacity;
} Queue;

// 函数声明
Graph* createGraph(int initCapacity, int isDirected);
void resizeGraph(Graph *g);
int addVertex(Graph *g, const char *name);
int findVertexIndex(Graph *g, const char *name);
void addEdge(Graph *g, int u, int v, int weight);
void addLineToStation(Graph *g, int stationIdx, int lineId);
void readMetroFile(const char *filename, Graph *g);
void printAdjList(Graph *g);

// 以下是需要实现的函数（TODO）
void DFSRecursive(Graph *g, int v, int *visited);
void DFSTraversal(Graph *g, int start);
void BFSTraversal(Graph *g, int start);
void connectivityAnalysis(Graph *g);
void dijkstra(Graph *g, int start, int *dist, int *prev);
void printPath(Graph *g, int *prev, int start, int end);
void shortestPathByTime(Graph *g, int start, int end);
void shortestPathByTransfer(Graph *g, int start, int end);
void freeGraph(Graph *g);

void printMenu();

// 队列操作
Queue* createQueue(int capacity);
void enqueue(Queue *q, int val);
int dequeue(Queue *q);
int isEmpty(Queue *q);
void freeQueue(Queue *q);

// ---------- 主函数 ----------
int main() {
    Graph *g = createGraph(100, 0);  // 无向图

    readMetroFile("metro.txt", g);

    int choice, start, end;
    char startName[MAX_NAME_LEN], endName[MAX_NAME_LEN];

    do {
        printMenu();
        printf("请输入选择：");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                printAdjList(g);
                break;
            case 2:
                printf("请输入起始站点名称：");
                fgets(startName, MAX_NAME_LEN, stdin);
                startName[strcspn(startName, "\n")] = '\0';
                start = findVertexIndex(g, startName);
                if (start == -1) {
                    fprintf(stderr, "错误：站点 '%s' 不存在。\n", startName);
                } else {
                    printf("\nDFS 遍历序列（从 %s 开始）：\n", startName);
                    DFSTraversal(g, start);
                }
                break;
            case 3:
                printf("请输入起始站点名称：");
                fgets(startName, MAX_NAME_LEN, stdin);
                startName[strcspn(startName, "\n")] = '\0';
                start = findVertexIndex(g, startName);
                if (start == -1) {
                    fprintf(stderr, "错误：站点 '%s' 不存在。\n", startName);
                } else {
                    printf("\nBFS 遍历序列（从 %s 开始）：\n", startName);
                    BFSTraversal(g, start);
                }
                break;
            case 4:
                connectivityAnalysis(g);
                break;
            case 5:
                printf("请输入起点站：");
                fgets(startName, MAX_NAME_LEN, stdin);
                startName[strcspn(startName, "\n")] = '\0';
                printf("请输入终点站：");
                fgets(endName, MAX_NAME_LEN, stdin);
                endName[strcspn(endName, "\n")] = '\0';
                start = findVertexIndex(g, startName);
                end = findVertexIndex(g, endName);
                if (start == -1) {
                    fprintf(stderr, "错误：起点 '%s' 不存在。\n", startName);
                } else if (end == -1) {
                    fprintf(stderr, "错误：终点 '%s' 不存在。\n", endName);
                } else {
                    shortestPathByTime(g, start, end);
                }
                break;
            case 6:
                printf("请输入起点站：");
                fgets(startName, MAX_NAME_LEN, stdin);
                startName[strcspn(startName, "\n")] = '\0';
                printf("请输入终点站：");
                fgets(endName, MAX_NAME_LEN, stdin);
                endName[strcspn(endName, "\n")] = '\0';
                start = findVertexIndex(g, startName);
                end = findVertexIndex(g, endName);
                if (start == -1) {
                    fprintf(stderr, "错误：起点 '%s' 不存在。\n", startName);
                } else if (end == -1) {
                    fprintf(stderr, "错误：终点 '%s' 不存在。\n", endName);
                } else {
                    shortestPathByTransfer(g, start, end);
                }
                break;
            case 0:
                printf("退出程序。\n");
                break;
            default:
                printf("无效选择，请重新输入。\n");
        }
        printf("\n");
    } while (choice != 0);

    freeGraph(g);
    return 0;
}

// ---------- 以下函数已完整实现（无需修改）----------

// 创建图
Graph* createGraph(int initCapacity, int isDirected) {
    Graph *g = (Graph*)malloc(sizeof(Graph));
    g->vertexCapacity = initCapacity;
    g->vertexNum = 0;
    g->edgeNum = 0;
    g->isDirected = isDirected;
    g->vertices = (VertexNode*)malloc(sizeof(VertexNode) * initCapacity);
    for (int i = 0; i < initCapacity; i++) {
        g->vertices[i].name[0] = '\0';
        g->vertices[i].firstEdge = NULL;
        g->vertices[i].lineIds = NULL;
        g->vertices[i].lineCount = 0;
    }
    return g;
}

// 动态扩容
void resizeGraph(Graph *g) {
    int newCap = g->vertexCapacity * 2;
    g->vertices = (VertexNode*)realloc(g->vertices, sizeof(VertexNode) * newCap);
    for (int i = g->vertexCapacity; i < newCap; i++) {
        g->vertices[i].name[0] = '\0';
        g->vertices[i].firstEdge = NULL;
        g->vertices[i].lineIds = NULL;
        g->vertices[i].lineCount = 0;
    }
    g->vertexCapacity = newCap;
}

// 添加站点，返回编号
int addVertex(Graph *g, const char *name) {
    int idx = findVertexIndex(g, name);
    if (idx != -1) return idx;

    if (g->vertexNum >= g->vertexCapacity) {
        resizeGraph(g);
    }
    strcpy(g->vertices[g->vertexNum].name, name);
    g->vertices[g->vertexNum].firstEdge = NULL;
    g->vertices[g->vertexNum].lineIds = NULL;
    g->vertices[g->vertexNum].lineCount = 0;
    return g->vertexNum++;
}

// 查找站点编号
int findVertexIndex(Graph *g, const char *name) {
    for (int i = 0; i < g->vertexNum; i++) {
        if (strcmp(g->vertices[i].name, name) == 0)
            return i;
    }
    return -1;
}

// 添加边（无向图加双向）
void addEdge(Graph *g, int u, int v, int weight) {
    if (u < 0 || u >= g->vertexNum || v < 0 || v >= g->vertexNum) return;

    EdgeNode *e = (EdgeNode*)malloc(sizeof(EdgeNode));
    e->adjVex = v;
    e->weight = weight;
    e->next = g->vertices[u].firstEdge;
    g->vertices[u].firstEdge = e;

    if (!g->isDirected) {
        e = (EdgeNode*)malloc(sizeof(EdgeNode));
        e->adjVex = u;
        e->weight = weight;
        e->next = g->vertices[v].firstEdge;
        g->vertices[v].firstEdge = e;
    }
    g->edgeNum++;
}

// 为站点添加所属线路编号
void addLineToStation(Graph *g, int stationIdx, int lineId) {
    if (stationIdx < 0 || stationIdx >= g->vertexNum) return;
    for (int i = 0; i < g->vertices[stationIdx].lineCount; i++) {
        if (g->vertices[stationIdx].lineIds[i] == lineId)
            return;
    }
    g->vertices[stationIdx].lineCount++;
    g->vertices[stationIdx].lineIds = (int*)realloc(g->vertices[stationIdx].lineIds,
                                                    sizeof(int) * g->vertices[stationIdx].lineCount);
    g->vertices[stationIdx].lineIds[g->vertices[stationIdx].lineCount - 1] = lineId;
}

// 读取地铁文件
void readMetroFile(const char *filename, Graph *g) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "无法打开文件 %s\n", filename);
        exit(1);
    }

    char line[256];
    int routeCount = 0;
    // 跳过前两行（总站点数和线路数，这里简单处理：读取直到遇到第一条线路数据）
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        sscanf(line, "%d", &routeCount);
        break;
    }
    // 再读一行（线路数）
    fgets(line, sizeof(line), fp);

    for (int rid = 0; rid < routeCount; rid++) {
        if (!fgets(line, sizeof(line), fp)) break;
        if (line[0] == '#' || line[0] == '\n') {
            rid--;
            continue;
        }

        char lineName[MAX_LINE_NAME];
        int stationCount;
        char *token = strtok(line, " \t\n");
        if (!token) continue;
        strcpy(lineName, token);

        token = strtok(NULL, " \t\n");
        if (!token) continue;
        stationCount = atoi(token);

        int prevStation = -1;
        int timeVal = 1;
        for (int i = 0; i < stationCount; i++) {
            token = strtok(NULL, " \t\n");
            if (!token) break;

            // 判断是否为纯数字（时间）
            int isTime = 1;
            for (char *p = token; *p; p++) {
                if (!isdigit(*p)) { isTime = 0; break; }
            }
            if (isTime && i > 0) {
                timeVal = atoi(token);
                continue;
            }

            int idx = addVertex(g, token);
            addLineToStation(g, idx, rid);

            if (prevStation != -1) {
                addEdge(g, prevStation, idx, timeVal);
                timeVal = 1;
            }
            prevStation = idx;
        }
    }
    fclose(fp);
    printf("成功读取地铁数据：共 %d 个站点，%d 条边。\n", g->vertexNum, g->edgeNum);
}

// 输出邻接表及换乘站
void printAdjList(Graph *g) {
    printf("\n===== 邻接表 =====\n");
    for (int i = 0; i < g->vertexNum; i++) {
        printf("%s (%d条线路): ", g->vertices[i].name, g->vertices[i].lineCount);
        EdgeNode *e = g->vertices[i].firstEdge;
        while (e) {
            printf("-> %s(%dmin) ", g->vertices[e->adjVex].name, e->weight);
            e = e->next;
        }
        printf("\n");
    }
    printf("\n===== 换乘站 =====\n");
    for (int i = 0; i < g->vertexNum; i++) {
        if (g->vertices[i].lineCount > 1) {
            printf("%s：%d 条线路\n", g->vertices[i].name, g->vertices[i].lineCount);
        }
    }
}

// 打印菜单
void printMenu() {
    printf("\n====== 地铁查询系统 ======\n");
    printf("1. 输出邻接表和换乘站\n");
    printf("2. DFS 遍历（从指定站点）\n");
    printf("3. BFS 遍历（从指定站点）\n");
    printf("4. 连通分量分析\n");
    printf("5. 最短路径（最少时间）\n");
    printf("6. 最短路径（最少换乘）\n");
    printf("0. 退出\n");
}

// ---------- 队列实现（已提供）----------
Queue* createQueue(int capacity) {
    Queue *q = (Queue*)malloc(sizeof(Queue));
    q->data = (int*)malloc(sizeof(int) * capacity);
    q->front = q->rear = q->size = 0;
    q->capacity = capacity;
    return q;
}

void enqueue(Queue *q, int val) {
    if (q->size == q->capacity) return;
    q->data[q->rear] = val;
    q->rear = (q->rear + 1) % q->capacity;
    q->size++;
}

int dequeue(Queue *q) {
    if (q->size == 0) return -1;
    int val = q->data[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    return val;
}

int isEmpty(Queue *q) {
    return q->size == 0;
}

void freeQueue(Queue *q) {
    free(q->data);
    free(q);
}
/**
 * DFS递归核心函数
 * @param g 图指针
 * @param v 当前访问顶点下标
 * @param visited 访问标记数组
 */
void DFSRecursive(Graph *g, int v, int *visited) {
    visited[v] = 1;
    printf("%s ", g->vertices[v].name);
    EdgeNode *e = g->vertices[v].firstEdge;
    while (e != NULL) {
        int neighbor = e->adjVex;
        if (!visited[neighbor]) {
            DFSRecursive(g, neighbor, visited);
        }
        e = e->next;
    }
}

/**
 * DFS遍历入口函数，初始化访问数组并启动递归
 */
void DFSTraversal(Graph *g, int start) {
    int vertexTotal = g->vertexNum;
    int *visited = (int*)calloc(vertexTotal, sizeof(int));
    DFSRecursive(g, start, visited);
    printf("\n");
    free(visited);
}

/**
 * BFS广度优先遍历，队列实现
 */
void BFSTraversal(Graph *g, int start) {
    int vertexTotal = g->vertexNum;
    int *visited = (int*)calloc(vertexTotal, sizeof(int));
    Queue *q = createQueue(vertexTotal);

    enqueue(q, start);
    visited[start] = 1;

    while (!isEmpty(q)) {
        int cur = dequeue(q);
        printf("%s ", g->vertices[cur].name);
        EdgeNode *e = g->vertices[cur].firstEdge;
        while (e != NULL) {
            int neighbor = e->adjVex;
            if (!visited[neighbor]) {
                visited[neighbor] = 1;
                enqueue(q, neighbor);
            }
            e = e->next;
        }
    }
    printf("\n");
    freeQueue(q);
    free(visited);
}

/**
 * 连通分量分析：统计所有连通块并输出每个块的站点
 */
void connectivityAnalysis(Graph *g) {
    int vertexTotal = g->vertexNum;
    int *visited = calloc(vertexTotal, sizeof(int));
    int compCount = 0;
    printf("\n===== 连通分量分析结果 =====\n");

    for (int i = 0; i < vertexTotal; ++i) {
        if (!visited[i]) {
            compCount++;
            Queue *q = createQueue(vertexTotal);
            enqueue(q, i);
            visited[i] = 1;
            printf("第%d个连通分量站点：", compCount);

            while (!isEmpty(q)) {
                int cur = dequeue(q);
                printf("%s ", g->vertices[cur].name);
                EdgeNode *e = g->vertices[cur].firstEdge;
                while (e) {
                    int neighbor = e->adjVex;
                    if (!visited[neighbor]) {
                        visited[neighbor] = 1;
                        enqueue(q, neighbor);
                    }
                    e = e->next;
                }
            }
            freeQueue(q);
            printf("\n");
        }
    }
    printf("图中总连通分量数量：%d\n", compCount);
    free(visited);
}

/**
 * Dijkstra算法：求解起点到所有点最短时间
 * @param dist 输出距离数组，dist[i]为起点到i最短时间
 * @param prev 输出前驱数组，prev[i]为路径上i的前一个站点下标
 */
void dijkstra(Graph *g, int start, int *dist, int *prev) {
    int n = g->vertexNum;
    int *finalized = (int*)calloc(n, sizeof(int));

    // 初始化距离无穷大、前驱-1
    for (int i = 0; i < n; i++) {
        dist[i] = INT_MAX;
        prev[i] = -1;
    }
    dist[start] = 0;

    for (int i = 0; i < n; i++) {
        // 选出未确定最短路径、距离最小的顶点u
        int u = -1;
        int minDis = INT_MAX;
        for (int j = 0; j < n; j++) {
            if (!finalized[j] && dist[j] < minDis) {
                minDis = dist[j];
                u = j;
            }
        }
        if (u == -1) break; // 剩余顶点不可达，提前退出
        finalized[u] = 1;

        // 松弛操作更新邻接点距离
        EdgeNode *e = g->vertices[u].firstEdge;
        while (e != NULL) {
            int v = e->adjVex;
            int w = e->weight;
            // 防int溢出，使用long long临时计算
            if (!finalized[v] && dist[u] != INT_MAX && (long long)dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                prev[v] = u;
            }
            e = e->next;
        }
    }
    free(finalized);
}

/**
 * 递归打印start到end的完整路径
 */
void printPath(Graph *g, int *prev, int start, int end) {
    if (end == start) {
        printf("%s", g->vertices[end].name);
        return;
    }
    printPath(g, prev, start, prev[end]);
    printf(" -> %s", g->vertices[end].name);
}

/**
 * 查询最少时间路径，调用Dijkstra并格式化输出
 */
void shortestPathByTime(Graph *g, int start, int end) {
    int n = g->vertexNum;
    int *dist = (int*)malloc(sizeof(int) * n);
    int *prev = (int*)malloc(sizeof(int) * n);

    dijkstra(g, start, dist, prev);

    if (dist[end] == INT_MAX) {
        printf("不存在从 %s 到 %s 的可达路径！\n", g->vertices[start].name, g->vertices[end].name);
    } else {
        printf("最短时间路径（总时间 %d 分钟）：", dist[end]);
        printPath(g, prev, start, end);
        printf("\n");
    }
    free(dist);
    free(prev);
}

/**
 * 查询最少换乘路径：临时将所有边权置1，用Dijkstra求最少边数（换乘=边数-1），结束后恢复原权值
 */
void shortestPathByTransfer(Graph *g, int start, int end) {
    int n = g->vertexNum;
    // 链表备份所有边原始权重
    typedef struct EdgeBackup {
        EdgeNode *edge;
        int oldWeight;
        struct EdgeBackup *next;
    } EdgeBackup;
    EdgeBackup *bkHead = NULL;

    // 遍历所有边，保存原始权值并修改权值为1
    for (int i = 0; i < n; i++) {
        EdgeNode *e = g->vertices[i].firstEdge;
        while (e) {
            EdgeBackup *newBk = (EdgeBackup*)malloc(sizeof(EdgeBackup));
            newBk->edge = e;
            newBk->oldWeight = e->weight;
            newBk->next = bkHead;
            bkHead = newBk;
            e->weight = 1;
            e = e->next;
        }
    }

    // 计算最少边路径
    int *dist = (int*)malloc(sizeof(int) * n);
    int *prev = (int*)malloc(sizeof(int) * n);
    dijkstra(g, start, dist, prev);

    // 输出结果
    if (dist[end] == INT_MAX) {
        printf("不存在从 %s 到 %s 的可达路径！\n", g->vertices[start].name, g->vertices[end].name);
    } else {
        int transferCount = dist[end] - 1;
        printf("最少换乘路径（换乘 %d 次）：", transferCount);
        printPath(g, prev, start, end);
        printf("\n");
    }

    // 恢复所有边原始权重，释放备份链表
    EdgeBackup *p = bkHead;
    while (p) {
        p->edge->weight = p->oldWeight;
        EdgeBackup *tmp = p;
        p = p->next;
        free(tmp);
    }

    free(dist);
    free(prev);
}

/**
 * 完整释放图所有动态内存，无内存泄漏
 */
void freeGraph(Graph *g) {
    if (g == NULL) return;
    // 释放每个顶点的边链表、线路数组
    for (int i = 0; i < g->vertexNum; i++) {
        // 释放邻接边
        EdgeNode *e = g->vertices[i].firstEdge;
        while (e != NULL) {
            EdgeNode *tmp = e;
            e = e->next;
            free(tmp);
        }
        // 释放线路id动态数组
        free(g->vertices[i].lineIds);
    }
    // 释放顶点数组
    free(g->vertices);
    // 释放图结构体本身
    free(g);
}