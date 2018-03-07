#include "meshpos.h"

node_t init_node(void){
    node_t d;
    memset(&d, 0, sizeof(node_t));
    return d;
}

line_t init_line(void){
    line_t d;
    memset(&d, 0, sizeof(line_t));
    return d;
}

triangle_t init_triangle(void){
    triangle_t d;
    memset(&d, 0, sizeof(triangle_t));
    return d;
}

void testRead(node_t **node, line_t **line){
    /* "a-b -38/-37;"
       "a-c -51/-54;"
       "b-c -43/-42;"
       "d-b -43/-42;"
       "c-b -43/-42;"
    */
    unsigned int i;
    unsigned int lineSize = 5, nodeSize = 4;
    *node = (node_t *)malloc(nodeSize * sizeof(node_t));
    *line = (line_t *)malloc(lineSize * sizeof(line_t));
    for(i=0;i<nodeSize;i++)
        (*node)[i] = init_node();
    for(i=0;i<lineSize;i++)
        (*line)[i] = init_line();

    strcpy((*node)[0].bssid,"a");
    strcpy((*node)[1].bssid,"b");
    strcpy((*node)[2].bssid,"c"); //(*node)[2] == node[0][2]
    strcpy((*node)[3].bssid,"d");
    (*node)[0].bssid_tag = 0;
    (*node)[1].bssid_tag = 1;
    (*node)[2].bssid_tag = 2;
    (*node)[3].bssid_tag = 3;

    (*line)[0].node1 = &(*node)[0]; //&(*node)[0] == &(*node)[0][0]
    (*line)[0].node2 = &(*node)[1];
    (*line)[0].rssi_1 = -38;
    (*line)[0].rssi_2 = -37;
    (*line)[0].distance = 5;

    (*line)[1].node1 = &(*node)[0];
    (*line)[1].node2 = &(*node)[2];
    (*line)[1].rssi_1 = -51;
    (*line)[1].rssi_2 = -54;
    (*line)[1].distance = 4.53;

    (*line)[2].node1 = &(*node)[1];
    (*line)[2].node2 = &(*node)[2];
    (*line)[2].rssi_1 = -43;
    (*line)[2].rssi_2 = -42;
    (*line)[2].distance = 2.3;

    (*line)[3].node1 = &(*node)[3];
    (*line)[3].node2 = &(*node)[1];
    (*line)[3].rssi_1 = -43;
    (*line)[3].rssi_2 = -42;
    (*line)[3].distance = 4.3;

    (*line)[4].node1 = &(*node)[2];
    (*line)[4].node2 = &(*node)[3];
    (*line)[4].rssi_1 = -43;
    (*line)[4].rssi_2 = -42;
    (*line)[4].distance = 4.15;
}

void marker(triangle_t triangle){
    triangle.ab->flag = 1;
    triangle.ac->flag = 1;
    triangle.bc->flag = 1;
    triangle.ab->node1->flag = 1;
    triangle.ab->node2->flag = 1;
    triangle.ac->node1->flag = 1;
    triangle.ac->node2->flag = 1;
}

void rssi2dist(line_t *lines, unsigned int size){
    /* 5G 與 2.4G 方程式不同 */
    float rssi;
    unsigned int i;

    //----------5G-----------
    const float P0 = -33.0;
    const float n = 1.90;

    for(i=0;i<size;i++){
        rssi = (float)(lines[i].rssi_1 + lines[i].rssi_2) / 2.0;
        lines[i].distance = pow(10.0, (P0-rssi)/(10.0*n));
        lines[i].rssi_merge = rssi;
    }
}

void dist2coor(triangle_t triangle){
    float d01=0.0, d02=0.0, d12=0.0;
    float alpha=0.0, cosine=0.0;
    node_t *a,*b,*c;

    //a coor (0,0)
    a = triangle.ab->node1;
    a->point.X = 0;
    a->point.Y = 0;

    //b coor (ab,0)
    b = triangle.ab->node2;
    b->point.X =  triangle.ab->distance;
    b->point.Y = 0;

    //c coor calc
    /*
    //define
    d01 = 4.3; //ab (x axis)
    d02 = 3.23; //ac (top-left)
    d12 = 4; //bc (top-right)
    */
    d01 = triangle.ab->distance; //ab
    d02 = triangle.ac->distance; //ac
    d12 = triangle.bc->distance; //bc
    cosine = (pow(d01,2) + pow(d02,2) - pow(d12,2)) / (2*d01*d02);

    //長度失焦(兩邊之和 < 第三邊)
    if(cosine > 1)
        cosine = 1;
    else if(cosine < -1)
        cosine = -1;
    alpha = acos(cosine);

    //警告
    //if(cosine == 1 || cosine == -1)
    //printf("ERROR:\n5G: Triangle side out of focus!\n");

    //找出c點(與a 不一樣的就是c點)
    if(triangle.ac->node1 == a)
        c = triangle.ac->node2;
    else
        c = triangle.ac->node1;

    //set
    c->point.X = (float)(d02*cos(alpha));
    c->point.Y = (float)(d02*sin(alpha));

    return;

    printf("x=%f\n",(float)(d02*cos(alpha)));
    printf("y=%f\n",(float)(d02*sin(alpha)));
    //----------------------------------
    //利用畢氏定理去補，或是直接 y 給 0.5 之類的小值
    //printf("(%f ,%f)\n", waps[2].X,waps[2].Y);
}

float dir_angle (point_t a,point_t b){
    /* 方向角，a :center */
    float angle = atan( fabs(b.X-a.X) / fabs(b.Y-a.Y) ) * (180.0/M_PI);
    if(b.X > a.X && b.Y < a.Y)
        angle = 180.0 - angle;
    else if(b.X < a.X && b.Y < a.Y)
        angle += 180.0;
    else if(b.X < a.X && b.Y > a.Y)
        angle = 360.0 - angle;
    return angle;
}

point_t rotate_coor(point_t p, float angle, point_t center){
    /* center 為旋轉圓心點，預設應為 0,0 */
    point_t newP;

    angle = (angle/180.0)*M_PI;
    p.X = p.X - center.X;
    p.Y = p.Y - center.Y;
    newP.X = (p.X * cos(angle)) - (p.Y * sin(angle));
    newP.Y = (p.X * sin(angle)) + (p.Y * cos(angle));
    newP.X = newP.X + center.X;
    newP.Y = newP.Y + center.Y;

    return newP;
}

float corner_angle(point_t A, point_t B, point_t C){
    //角ABC 的夾角角度(三角形夾角不會大於 180度)
    float a = sqrt(pow(B.X-A.X,2) + pow(B.Y-A.Y,2));
    float b = sqrt(pow(C.X-B.X,2) + pow(C.Y-B.Y,2));
    float c = sqrt(pow(C.X-A.X,2) + pow(C.Y-A.Y,2));
    //COSX=(a^2+b^2-c^2)/(2ab)
    float angle = (pow(a,2) + pow(b,2) - pow(c,2)) / (2*a*b);
    return acos(angle) * (180.0/M_PI);
}

void linkSide(line_t *workline, line_t *line1, line_t *line2){
    /* save clone 用 */
    line_t ab,ac,bc;
    node_t a,b,c;
    triangle_t t_clone; // clone triangle

    /* find "d" 用  (d:三角形非連接邊的 那點) */
    line_t *fixedMarkerLine = NULL, *cloneMarkerLine = NULL;
    node_t *d = NULL;

    /* 計算連接邊 後的 clone triangle 座標 */
    point_t D;


    //相對應的line若有一組有已勾(連邊)
    if(line1->flag ^ line2->flag){

        //copy and 整理(t_clone 整理後 邊與邊可能會對調，所以判斷方式不能依原本的方式)
        // 如果沒有 copy, dist2coor() 計算會直接改掉原本的 bc 值
        ab = *workline; //by value
        ac = *line1;
        bc = *line2;
        a = *ab.node1; //by value
        b = *ab.node2;
        //搜尋c，有一個比對一樣就不會是c
        if(ab.node1->bssid_tag == ac.node1->bssid_tag || ab.node2->bssid_tag == ac.node1->bssid_tag)
            c = *ac.node2;
        else
            c = *ac.node1;
        ab.node1 = &a;
        ab.node2 = &b;
        ac.node1 = &a;
        ac.node2 = &c;
        bc.node1 = &b;
        bc.node2 = &c;

        //calc
        t_clone.ab = &ab;
        t_clone.ac = &ac;
        t_clone.bc = &bc;
        dist2coor(t_clone);

        //find marker line
        if(line1->flag)
            fixedMarkerLine = line1;
        else
            fixedMarkerLine = line2;
        //因整理後邊可能會對調，所以尋找方式是: dist與marker line(fixed) 一樣的 line才是marker line(clone)
        if(fixedMarkerLine->distance == ac.distance)
            cloneMarkerLine = &ac;
        else if(fixedMarkerLine->distance == bc.distance)
            cloneMarkerLine = &bc;
        else if(fixedMarkerLine->distance == ab.distance)
            cloneMarkerLine = &ab;

        //find  clone triangle 的 "d"
        if(a.bssid_tag != cloneMarkerLine->node1->bssid_tag && a.bssid_tag != cloneMarkerLine->node2->bssid_tag)
            d = &a;
        else if (b.bssid_tag != cloneMarkerLine->node1->bssid_tag && b.bssid_tag != cloneMarkerLine->node2->bssid_tag)
            d = &b;
        else if (c.bssid_tag != cloneMarkerLine->node1->bssid_tag && c.bssid_tag != cloneMarkerLine->node2->bssid_tag)
            d = &c;

        //Calculate the coordinates of the linked side clone triangle
        D.X = d->point.X + fixedMarkerLine->node1->point.X;
        D.Y = d->point.Y + fixedMarkerLine->node1->point.Y;
        //旋轉中心都固定定義 node1 (如果 fixed 與 clone 的 angle 定的旋轉中心不一，頂多呈現鏡像，後面會再判斷所以還好)
        float fixedMarkerAngle = dir_angle(fixedMarkerLine->node1->point, fixedMarkerLine->node2->point);
        float cloneMarkerAngle = dir_angle(cloneMarkerLine->node1->point,cloneMarkerLine->node2->point);
        float rotateAngle = cloneMarkerAngle - fixedMarkerAngle; //因 負為順時鐘，所以一定要 clone - fixed (動 - 固定)
        D = rotate_coor(D, rotateAngle, fixedMarkerLine->node1->point); //B-C 座標原本就要沿用 fixed(因為連接邊)，所以計算出 D 即可
    }
}




















