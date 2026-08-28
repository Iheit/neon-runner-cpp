#include "asset_generator.h"
#include <raylib.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <string>
#include <vector>

namespace {
constexpr int W=1280,H=720;
constexpr float LANE=3.2f,PLAYER_Z=2.5f,SEG=14.0f;
constexpr int SEGMENTS=22;

struct Segment { float z=0; int obstacleLane=0,coinLane=1,style=0; float obstacleHeight=2; bool coinCollected=false; };
float LaneX(int lane){return (lane-1)*LANE;}
float Rand(float a,float b){return a+float(std::rand())/float(RAND_MAX)*(b-a);}
void Generate(Segment& s,float z){s.z=z;s.obstacleLane=std::rand()%3;s.coinLane=(s.obstacleLane+1+std::rand()%2)%3;s.style=std::rand()%6;s.obstacleHeight=Rand(1.7f,2.8f);s.coinCollected=false;}
Model Cube(float x,float y,float z){return LoadModelFromMesh(GenMeshCube(x,y,z));}
Model Cylinder(float r,float h,int sides=32){return LoadModelFromMesh(GenMeshCylinder(r,h,sides));}
Model Sphere(float r){return LoadModelFromMesh(GenMeshSphere(r,24,16));}

void Window(Vector3 p,float w,float h,Color c){
    DrawCube(p,w,h,0.045f,c);
    DrawCube({p.x,p.y,p.z-0.035f},w+0.025f,h+0.025f,0.025f,Color{120,130,160,255});
}
void Building(Vector3 p,float w,float h,float d,int style,bool left){
    Color shell=style%2?Color{32,28,55,255}:Color{23,34,57,255};
    Color glow=left?Color{55,220,255,255}:Color{245,70,215,255};
    DrawCube(p,w,h,d,shell);
    DrawCubeWires(p,w+.08f,h+.08f,d+.08f,Color{75,85,115,255});
    for(int floor=0;floor<int(h/1.55f);++floor){
        float y=p.y-h/2+.85f+floor*1.55f;
        for(int col=0;col<4;++col){
            float x=p.x-w/2+.62f+col*(w-1.24f)/3.0f;
            if((col+floor+style)%4!=0) Window({x,y,p.z-d/2-.035f},.42f,.48f,glow);
        }
    }
    for(int i=0;i<3;++i){
        float x=p.x-w/2+.45f+i*(w-.9f)/2;
        DrawCube({x,p.y+h/2+.08f,p.z},.14f,.16f,d*.72f,glow);
    }
    if(style%3==0){
        DrawCube({p.x,p.y+h/2+.38f,p.z},w*.55f,.10f,d*.48f,glow);
        DrawCube({p.x,p.y+h/2+.52f,p.z},w*.30f,.08f,d*.28f,Color{190,200,230,255});
    }
}
void Lamp(float z,int side){
    float x=side*5.15f; Color c=side<0?Color{60,230,255,255}:Color{245,75,220,255};
    DrawCylinder({x,1.65f,z},.075f,3.3f,12,Color{65,70,92,255});
    DrawCylinder({x,3.28f,z},.22f,.12f,16,c);
    DrawCube({x,3.18f,z},.42f,.08f,.42f,c);
}
void Barrier(const Segment& s){
    float x=LaneX(s.obstacleLane),h=s.obstacleHeight; Color red{245,55,85,255};
    if(s.style%3==0){
        DrawCube({x,h/2,s.z},1.85f,h,1.75f,Color{80,25,48,255});
        for(int i=0;i<5;++i) DrawCube({x-.7f+i*.35f,h*.55f,s.z-.91f},.16f,.16f,.05f,red);
        DrawCubeWires({x,h/2,s.z},1.92f,h+.08f,1.82f,Color{255,145,165,255});
    }else if(s.style%3==1){
        DrawCylinder({x,h/2,s.z},.92f,h,10,Color{45,48,70,255});
        DrawCylinder({x,h*.72f,s.z},.72f,.13f,24,red);
        DrawCylinder({x,h*.38f,s.z},.60f,.10f,24,Color{255,175,185,255});
    }else{
        DrawCube({x,h/2,s.z},1.75f,h,1.7f,Color{36,40,60,255});
        for(int i=0;i<4;++i) DrawCube({x-.62f+i*.41f,h*.55f,s.z-.88f},.18f,h*.50f,.06f,red);
        DrawCube({x,h+.10f,s.z},1.95f,.12f,1.95f,Color{255,120,145,255});
    }
}
void Coin(Vector3 p){
    float r=float(GetTime())*180.0f;
    DrawCylinder(p,.48f,.12f,32,Color{255,205,40,255});
    DrawTorus(p,.29f,.055f,20,12,Color{255,245,140,255});
    DrawCylinderWires(p,.51f,.14f,32,Color{255,250,180,255});
    (void)r;
}
void SegmentDraw(const Segment& s,Model road){
    DrawModel(road,{0,-.1f,s.z},1,Color{43,48,65,255});
    float lh=6.0f+s.style*1.15f,rh=7.0f+(5-s.style)*.9f;
    Building({-7,lh/2,s.z},4.3f,lh,11.5f,s.style,true);
    Building({7,rh/2,s.z+1},4.5f,rh,11.5f,(s.style+2)%6,false);
    Lamp(s.z,-1); Lamp(s.z,1);
    DrawCube({-5.25f,2.8f,s.z},.10f,5.5f,.10f,Color{60,225,255,255});
    DrawCube({5.25f,2.8f,s.z},.10f,5.5f,.10f,Color{245,75,215,255});
    DrawCube({-5.25f,5.5f,s.z},1.0f,.10f,.14f,Color{60,225,255,255});
    DrawCube({5.25f,5.5f,s.z},1.0f,.10f,.14f,Color{245,75,215,255});
    Barrier(s);
    if(!s.coinCollected){float bob=std::sin(float(GetTime())*4+s.z)*.18f;Coin({LaneX(s.coinLane),1.45f+bob,s.z-4});}
}
void Player(Model torso,Model head,Model limb,Model boot,Vector3 p,float t,bool jump){
    float run=std::sin(t*12),bob=jump?0:std::fabs(run)*.06f,y=p.y+bob;
    Color suit{45,175,220,255},glow{70,225,255,255},dark{24,48,82,255};
    DrawModel(torso,{p.x,y+.72f,p.z},1,suit);
    DrawModel(head,{p.x,y+1.58f,p.z},1,Color{105,230,255,255});
    DrawSphere({p.x,y+1.58f,p.z-.43f},.11f,glow);
    DrawCube({p.x,y+.77f,p.z-.55f},.50f,.40f,.05f,dark);
    DrawCube({p.x,y+.88f,p.z-.59f},.28f,.07f,.04f,glow);
    float a=run*.34f,b=-a;
    DrawModelEx(limb,{p.x-.27f+a,y-.30f,p.z},{0,0,1},run*18,{.70f,1,.75f},dark);
    DrawModelEx(limb,{p.x+.27f+b,y-.30f,p.z},{0,0,1},-run*18,{.70f,1,.75f},dark);
    DrawModelEx(boot,{p.x-.27f+a*1.5f,y-.72f,p.z-.18f},{0,0,1},0,{.9f,.30f,1.35f},glow);
    DrawModelEx(boot,{p.x+.27f+b*1.5f,y-.72f,p.z-.18f},{0,0,1},0,{.9f,.30f,1.35f},glow);
    DrawModelEx(limb,{p.x-.60f-b,y+.60f,p.z},{0,0,1},-run*16,{1.05f,.30f,.55f},suit);
    DrawModelEx(limb,{p.x+.60f-a,y+.60f,p.z},{0,0,1},run*16,{1.05f,.30f,.55f},suit);
    DrawSphere({p.x-.84f-b,y+.57f,p.z},.17f,glow);
    DrawSphere({p.x+.84f-a,y+.57f,p.z},.17f,glow);
}
}

int main(){
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    InitWindow(W,H,"Neon Runner"); SetTargetFPS(60);
    int progress=0; std::string status="Generating procedural assets...";
    GenerateAssets(std::filesystem::current_path().string(),[&](int v,const std::string& s){
        progress=v;status=s;BeginDrawing();ClearBackground(Color{7,10,20,255});
        DrawText("NEON RUNNER",440,220,48,Color{100,225,255,255});DrawText(status.c_str(),430,315,22,RAYWHITE);
        DrawRectangle(360,365,560,20,Color{30,35,55,255});DrawRectangle(360,365,560*progress/100,20,Color{70,220,255,255});
        DrawText(TextFormat("%d%%",progress),610,405,20,RAYWHITE);EndDrawing();
    });
    std::string root=(std::filesystem::current_path()/"assets"/"generated").string(); InitAudioDevice();
    Sound coinSound=LoadSound((root+"/coin.wav").c_str()),jumpSound=LoadSound((root+"/jump.wav").c_str());
    Sound hitSound=LoadSound((root+"/hit.wav").c_str()),stepSound=LoadSound((root+"/step.wav").c_str());
    Model road=Cube(10,.18f,SEG),torso=Cylinder(.55f,1.3f),head=Sphere(.48f),limb=Cube(.55f,.75f,.55f),boot=Cube(.4f,.25f,.4f);
    Camera3D camera{};camera.position={0,6.7f,11};camera.target={0,1.3f,-12};camera.up={0,1,0};camera.fovy=58;camera.projection=CAMERA_PERSPECTIVE;
    std::vector<Segment> segments(SEGMENTS);for(int i=0;i<SEGMENTS;++i)Generate(segments[i],-i*SEG);
    int lane=1,score=0;float py=1,vy=0,speed=18,distance=0,runTime=0,stepTimer=0;bool over=false;
    auto reset=[&](){lane=1;score=0;py=1;vy=0;speed=18;distance=0;runTime=0;stepTimer=0;over=false;for(int i=0;i<SEGMENTS;++i)Generate(segments[i],-i*SEG);};
    while(!WindowShouldClose()){
        float dt=std::min(GetFrameTime(),.033f);
        if(!over){
            if(IsKeyPressed(KEY_A)||IsKeyPressed(KEY_LEFT))lane=std::max(0,lane-1);
            if(IsKeyPressed(KEY_D)||IsKeyPressed(KEY_RIGHT))lane=std::min(2,lane+1);
            if(IsKeyPressed(KEY_SPACE)&&py<=1.01f){vy=10;PlaySound(jumpSound);}
            vy-=25*dt;py+=vy*dt;if(py<1){py=1;vy=0;}
            speed=std::min(42.0f,speed+.42f*dt);distance+=speed*dt;runTime+=dt;stepTimer+=dt;score=int(distance*1.25f);
            if(stepTimer>.38f&&py<=1.05f){PlaySound(stepSound);stepTimer=0;}
            float farthest=0;for(auto&s:segments)farthest=std::min(farthest,s.z);for(auto&s:segments)s.z+=speed*dt;
            for(auto&s:segments)if(s.z>22){s.z=farthest-SEG;Generate(s,s.z);farthest=s.z;}
            Vector3 pp={LaneX(lane),py,PLAYER_Z};BoundingBox pb={{pp.x-.55f,pp.y-.75f,pp.z-.45f},{pp.x+.55f,pp.y+.8f,pp.z+.45f}};
            for(auto&s:segments){
                if(s.obstacleLane==lane&&std::fabs(s.z-PLAYER_Z)<1.15f){BoundingBox ob={{LaneX(lane)-.9f,0,s.z-.9f},{LaneX(lane)+.9f,s.obstacleHeight,s.z+.9f}};if(CheckCollisionBoxes(pb,ob)){over=true;PlaySound(hitSound);}}
                float cz=s.z-4;if(!s.coinCollected&&s.coinLane==lane&&std::fabs(cz-PLAYER_Z)<.95f&&std::fabs(py-1.45f)<1){s.coinCollected=true;score+=25;PlaySound(coinSound);}
            }
        }else if(IsKeyPressed(KEY_R))reset();
        BeginDrawing();ClearBackground(Color{7,10,20,255});BeginMode3D(camera);
        for(const auto&s:segments)SegmentDraw(s,road);Player(torso,head,limb,boot,{LaneX(lane),py,PLAYER_Z},runTime,py>1.05f);EndMode3D();
        DrawRectangle(0,0,W,78,Color{5,8,18,225});DrawText("NEON RUNNER",28,18,28,Color{100,225,255,255});
        DrawText(TextFormat("SCORE %05d",score),275,20,26,RAYWHITE);DrawText(TextFormat("SPEED %.1f",speed),480,22,20,Color{175,185,210,255});
        DrawText("A/D MOVE   SPACE JUMP",900,24,17,Color{175,185,210,255});
        if(over){DrawRectangle(0,0,W,H,Color{5,7,15,175});DrawText("SYSTEM FAILURE",450,255,54,Color{255,90,110,255});DrawText(TextFormat("SCORE %d",score),565,330,25,RAYWHITE);DrawText("PRESS R TO RESTART",505,390,22,Color{150,220,255,255});}
        EndDrawing();
    }
    UnloadModel(road);UnloadModel(torso);UnloadModel(head);UnloadModel(limb);UnloadModel(boot);UnloadSound(coinSound);UnloadSound(jumpSound);UnloadSound(hitSound);UnloadSound(stepSound);CloseAudioDevice();CloseWindow();return 0;
}
