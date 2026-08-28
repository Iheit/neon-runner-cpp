#include "asset_generator.h"
#include <raylib.h>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <cstdlib>
#include <ctime>
#include <vector>

struct Obstacle { Vector3 p; float w=1.8f,h=2.0f,d=2.0f; };
struct Coin { Vector3 p; bool got=false; float t=0; };
static float laneX(int l){ return (l-1)*3.2f; }
static float rnd(float a,float b){ return a + float(std::rand())/float(RAND_MAX)*(b-a); }

int main(){
 std::srand(unsigned(std::time(nullptr)));
 InitWindow(1280,720,"Neon Runner"); SetTargetFPS(60);
 bool loading=true; int loadPct=0; const char* loadText="Starting...";
 std::string root=std::filesystem::current_path().string();
 GenerateAssets(root,[&](int p,const std::string& s){loadPct=p; loadText=s.c_str(); BeginDrawing(); ClearBackground({7,10,20,255}); DrawText("NEON RUNNER",440,220,48,{100,225,255,255}); DrawText(loadText,460,315,22,RAYWHITE); DrawRectangle(360,365,560,20,{30,35,55,255}); DrawRectangle(360,365,560*loadPct/100,20,{70,220,255,255}); DrawText(TextFormat("%d%%",loadPct),610,405,20,RAYWHITE); EndDrawing(); });
 loading=false; (void)loading;
 Camera3D cam{{0,7,10.5f},{0,1.1f,-8},{0,1,0},55,CAMERA_PERSPECTIVE};
 int lane=1,score=0; float y=1,vy=0,speed=18,dist=0; bool dead=false;
 std::vector<Obstacle> obs; std::vector<Coin> coins;
 auto reset=[&](){lane=1;score=0;y=1;vy=0;speed=18;dist=0;dead=false;obs.clear();coins.clear();for(int i=0;i<10;i++){float z=-18-i*15.0f;if(i%2==0)obs.push_back({{laneX((i+1)%3),1,z}});coins.push_back({{laneX((i+2)%3),1.3f,z-5},false,rnd(0,6.28f)});}}; reset();
 while(!WindowShouldClose()){
  float dt=std::min(GetFrameTime(),.033f);
  if(!dead){
   if(IsKeyPressed(KEY_A)||IsKeyPressed(KEY_LEFT)) lane=std::max(0,lane-1); if(IsKeyPressed(KEY_D)||IsKeyPressed(KEY_RIGHT)) lane=std::min(2,lane+1);
   if(IsKeyPressed(KEY_SPACE)&&y<=1.01f)vy=10;
   vy-=25*dt;y+=vy*dt;if(y<1){y=1;vy=0;} speed=std::min(34.f,speed+.55f*dt);dist+=speed*dt;score=int(dist*1.25f);
   for(auto&o:obs)o.p.z+=speed*dt;for(auto&c:coins){c.p.z+=speed*dt;c.t+=dt*5;}
   float far=-100;for(auto&o:obs)far=std::min(far,o.p.z);for(auto&c:coins)far=std::min(far,c.p.z);if(far>-100){float z=far-rnd(24,34);int ol=std::rand()%3;obs.push_back({{laneX(ol),1,z},1.8f,rnd(1.7f,2.5f),2});int cl=(ol+1+std::rand()%2)%3;coins.push_back({{laneX(cl),rnd(1.2f,1.8f),z-5},false,rnd(0,6.28f)});}
   Vector3 pp{laneX(lane),y,2.5f};BoundingBox pb{{pp.x-.62f,pp.y-.68f,pp.z-.62f},{pp.x+.62f,pp.y+.68f,pp.z+.62f}};
   for(auto&o:obs){BoundingBox b{{o.p.x-o.w/2,o.p.y-o.h/2,o.p.z-o.d/2},{o.p.x+o.w/2,o.p.y+o.h/2,o.p.z+o.d/2}};if(CheckCollisionBoxes(pb,b)){dead=true;break;}}
   for(auto&c:coins)if(!c.got&&fabs(c.p.x-pp.x)<.9f&&fabs(c.p.z-pp.z)<.9f&&fabs(c.p.y+sin(c.t*3)*.15f-pp.y)<1){c.got=true;score+=25;}
   obs.erase(std::remove_if(obs.begin(),obs.end(),[](auto&o){return o.p.z>18;}),obs.end());coins.erase(std::remove_if(coins.begin(),coins.end(),[](auto&c){return c.p.z>18||c.got;}),coins.end());
  }else if(IsKeyPressed(KEY_R))reset();
  BeginDrawing();ClearBackground({7,10,20,255});BeginMode3D(cam);
  DrawPlane({0,-.08f,0},{24,180},{20,24,38,255});
  DrawCube({-5.25f,.02f,-25},{.08f,.12f,180},{70,220,255,255});DrawCube({5.25f,.02f,-25},{.08f,.12f,180},{70,220,255,255});
  float off=fmod(dist*.9f,8.f);for(int d=0;d<2;d++){float x=d?-1.6f:1.6f;for(int i=-12;i<14;i++)DrawCube({x,.055f,i*8.f+off},{.09f,.04f,3.5f},{115,125,150,255});}
  for(int i=-9;i<=9;i+=2){float h=4+abs(i%5)*1.8f;DrawCube({i*3.6f,h/2,-45},{2.6f,h,2.6f},{24,28,48,255});}
  for(auto&o:obs){DrawCube(o.p,{o.w,o.h,o.d},{255,70,95,255});DrawCubeWires(o.p,{o.w+.05f,o.h+.05f,o.d+.05f},{255,150,165,255});}
  for(auto&c:coins)if(!c.got){float b=sin(c.t*3)*.15f;DrawCylinder({c.p.x,c.p.y+b,c.p.z},.48f,.1f,32,{255,220,70,255});}
  DrawCube({laneX(lane),y,2.5f},{1.45f,1.45f,1.45f},{70,220,255,255});DrawCubeWires({laneX(lane),y,2.5f},{1.5f,1.5f,1.5f},{170,245,255,255});EndMode3D();
  DrawRectangle(0,0,1280,78,{5,8,18,225});DrawText("NEON RUNNER",28,18,28,{100,225,255,255});DrawText(TextFormat("SCORE %05d",score),275,20,26,RAYWHITE);DrawText(TextFormat("SPEED %.1f",speed),480,22,20,{175,185,210,255});DrawText("A/D MOVE   SPACE JUMP",900,24,17,{175,185,210,255});
  if(dead){DrawRectangle(0,0,1280,720,{5,7,15,170});DrawText("SYSTEM FAILURE",450,255,54,{255,90,110,255});DrawText(TextFormat("SCORE %d",score),565,330,25,RAYWHITE);DrawText("PRESS R TO RESTART",505,390,22,{150,220,255,255});} EndDrawing();
 }
 CloseWindow(); return 0;
}
