#include "pointCloud.hpp"
#define CFG_PATH "cfg/cfg.yaml"
#include <algorithm>

using namespace std;

PointCloud pc;
YAML::Node cfg;

std::map<string,bool> key;
std::map<string,int> view;
std::map<string,float> cam;

void mouseMove(int x, int y);

void mouseClick(int button, int state, int x, int y);

void update(int n);

void renderScene();

void iniGl();

void keyModifiers(unsigned char k, int mod);

bool checkKey(unsigned char k, char* key);

void keyPressed(unsigned char k, int x, int y);

void keyReleased(unsigned char k, int x, int y);

void plotScale();

void plotAxis();

void plotSelector();

void heatMap(float input,float* r,float* g,float* b);


int main(int argc, char **argv){

	// Loading system configuration file
	cfg = YAML::LoadFile(CFG_PATH);

	// Processing derived parameters
	cfg["video"]["c_aspect"] = (float)cfg["video"]["width"].as<int>()/cfg["video"]["height"].as<int>();

	// Loading hotkeys
	for(YAML::const_iterator it=cfg["hotkeys"].begin(); it!=cfg["hotkeys"].end(); ++it)
		key[it->first.as<std::string>().c_str()]	= false;
	
	// Loading default view states
	for(YAML::const_iterator it=cfg["view"].begin(); it!=cfg["view"].end(); ++it)
		view[it->first.as<std::string>().c_str()]	= it->second.as<int>();

	// Loading default camera states
	for(YAML::const_iterator it=cfg["cam"].begin(); it!=cfg["cam"].end(); ++it)
		cam[it->first.as<std::string>().c_str()]	= it->second.as<float>();

	// Loading point cloud file
	if(pc.load())
		return 0;
	pc.compute_stats();

	glutInit(&argc, argv);
	iniGl();
	glutMainLoop();
	return 0;
}

void renderScene(){
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		gluLookAt(
			cam["ctr_x"]+cam["cam_x"],
			cam["ctr_y"]+cam["cam_y"],
			cam["ctr_z"]+cam["cam_z"],
			cam["ctr_x"],cam["ctr_y"],
			cam["ctr_z"],0,0,int(cam["upwards"]));

		// Points
		float r = 0.5;
		float g = 0.5;
		float b = 0.5;
		glPointSize(view["point_size"]);
		glBegin(GL_POINTS);
		for (int i = 0; i < pc.num_points; i++){
			if(cfg["viz"]["viz_modes"][view["viz_mode"]].as<string>().compare("ORIGINAL") == 0){
				r = pc.point[i].r/255;
				g = pc.point[i].g/255;
				b = pc.point[i].b/255;
			}
			if(cfg["viz"]["viz_modes"][view["viz_mode"]].as<string>().compare("HEAT_MAPZ") == 0)
				heatMap((pc.point[i].z-pc.minZ)/(pc.maxZ-pc.minZ),&r,&g,&b);
			if(cfg["viz"]["viz_modes"][view["viz_mode"]].as<string>().compare("HEAT_MAPY") == 0)
				heatMap((pc.point[i].y-pc.minY)/(pc.maxY-pc.minY),&r,&g,&b);
			glColor3f(r,g,b);
			glVertex3f(
				pc.point[i].x,
				pc.point[i].y,
				pc.point[i].z
			);
		}
		glEnd();
		
		// Axis
		if (view["plot_axis"])
			plotAxis();
		plotSelector();
		plotScale();

	glPopMatrix();
	glutSwapBuffers();
}

void iniGl(){
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(cfg["video"]["width"].as<int>(), cfg["video"]["height"].as<int>());
	glutCreateWindow(cfg["video"]["title"].as<string>().c_str());
	glutKeyboardFunc(&keyPressed);
	glutKeyboardUpFunc(&keyReleased);
	glutDisplayFunc(&renderScene);
	glutIdleFunc(&renderScene);
	glutPassiveMotionFunc(&mouseMove);
	glutMouseFunc(&mouseClick);
	if(cfg["video"]["background_color"].size() == 3)
		glClearColor(
			cfg["video"]["background_color"][0].as<float>(),
			cfg["video"]["background_color"][1].as<float>(),
			cfg["video"]["background_color"][2].as<float>(),
			0
		);
	else
		glClearColor(0.0,0.0,0.0,0);
	glMatrixMode(GL_PROJECTION);
	glutTimerFunc(1,update,0);
	glutSetCursor(GLUT_CURSOR_NONE);
	gluPerspective(
		cfg["rendering"]["vfov"].as<float>(),
		cfg["video"]["c_aspect"].as<float>(),
		cfg["rendering"]["z_near"].as<float>(),
		cfg["rendering"]["z_far"].as<float>()
	);
	if (cfg["video"]["full_screen"].as<bool>())
		glutFullScreen();
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
}

void mouseMove(int x, int y){

	int relMouseX	= x - cfg["video"]["width"].as<int>()/2;
	int relMouseY	= y - cfg["video"]["height"].as<int>()/2;
	int mouse_size	= cfg["camera"]["mouse_size"].as<float>();

	if(relMouseX > mouse_size)
		glutWarpPointer(cfg["video"]["width"].as<int>()/2 - mouse_size, y);
	else if(relMouseX < - mouse_size)
		glutWarpPointer(cfg["video"]["width"].as<int>()/2 + mouse_size, y);

	if(relMouseY > mouse_size)
		glutWarpPointer(x, cfg["video"]["height"].as<int>()/2 - mouse_size);
	else if(relMouseY < - mouse_size)
		glutWarpPointer(x, cfg["video"]["height"].as<int>()/2 + mouse_size);

	cam["theta"]	= 180*(double)relMouseX/mouse_size + 180;
	cam["phi"]		= 89*(double)relMouseY/mouse_size + 270;

}

void mouseClick(int button, int state, int x, int y){
	if(button == 0)
		cam["radius"] /= cfg["camera"]["zoom_factor_mouse"].as<float>();
	if(button == 2)
		cam["radius"] *= cfg["camera"]["zoom_factor_mouse"].as<float>();
	if(button == 3)
		cam["radius"] /= cfg["camera"]["zoom_factor_mouse"].as<float>();
	if(button == 4)
		cam["radius"] *= cfg["camera"]["zoom_factor_mouse"].as<float>();
}

void plotScale(){
	glPushMatrix();
		glLineWidth(2);
		glPushMatrix();
			if(cfg["camera"]["invert_top"].as<bool>())
				glTranslatef(pc.maxX-2,pc.maxY-4*pc.lenY/100,99);
			else
				glTranslatef(pc.maxX-2,pc.minY+4*pc.lenY/100,99);
			glBegin(GL_LINES);
				glVertex3f(0,0,0);
				glVertex3f(0,2*pc.lenY/100,0);
			glEnd();
		glPopMatrix();
		glPushMatrix();
			if(cfg["camera"]["invert_top"].as<bool>())
				glTranslatef(pc.maxX-12,pc.maxY-4*pc.lenY/100,99);
			else
				glTranslatef(pc.maxX-12,pc.minY+4*pc.lenY/100,99);
			glBegin(GL_LINES);
				glVertex3f(0,0,0);
				glVertex3f(0,2*pc.lenY/100,0);
			glEnd();
		glPopMatrix();
		glPushMatrix();
			if(cfg["camera"]["invert_top"].as<bool>())
				glTranslatef(pc.maxX-12,pc.maxY-3*pc.lenY/100,99);
			else
				glTranslatef(pc.maxX-12,pc.minY+5*pc.lenY/100,99);
			glColor3f(0.0,0.0,0.0);
			glBegin(GL_LINES);
				glVertex3f(0,0,0);
				glVertex3f(10,0,0);
			glEnd();
		glPopMatrix();
		glLineWidth(1);
	glPopMatrix();
}

void plotAxis(){
	glColor3f(1,0.5,0.5);
	glBegin(GL_LINES);
		glVertex3f(0,0,0);
		glVertex3f(1,0,0);
	glEnd();
	glColor3f(0.5,1,0.5);
	glBegin(GL_LINES);
		glVertex3f(0,0,0);
		glVertex3f(0,1,0);
	glEnd();
	glColor3f(0.5,0.5,1);
	glBegin(GL_LINES);
		glVertex3f(0,0,0);
		glVertex3f(0,0,1);
	glEnd();
}

void plotSelector(){
	glPushMatrix();
		glColor4f(0.5,0.5,0.5,0.5);
		glTranslatef(cam["ctr_x"],cam["ctr_y"],cam["ctr_z"]);
		glRotatef(view["selector_angle"],0,0,1);
		glutSolidCube(0.1);
	glPopMatrix();
}

void heatMap(float input,float* r,float* g,float* b){
	if(input < 0.25){
		*r = 0;
		*g = 4*input;
		*b = 1;
	}else if(input < 0.50){
		*r = 0;
		*g = 1;
		*b = 1.0-4*(input-0.25);		
	}else if(input < 0.75){
		*r = 4*(input-0.50);
		*g = 1;
		*b = 0;
	}else{
		*r = 1;
		*g = 1.0-4*(input-0.75);
		*b = 0;
	}
}

void update(int n){
	glutTimerFunc(1,update,0);
	if(key["zoom_in"])
		cam["radius"] /= cfg["camera"]["zoom_factor_key"].as<float>();
	if(key["zoom_out"])
		cam["radius"] *= cfg["camera"]["zoom_factor_key"].as<float>();
	cam["cam_x"]	= cam["radius"]*sin(M_PI*cam["theta"]/180.0)*sin(M_PI*cam["phi"]/180.0);
	cam["cam_y"]	= cam["radius"]*cos(M_PI*cam["theta"]/180.0)*sin(M_PI*cam["phi"]/180.0);
	cam["cam_z"]	= cam["radius"]*cos(M_PI*cam["phi"]/180.0);
	cam["upwards"]	= 2*(cam["phi"]>180)-1;
	float cam_vel = (key["fast_cam"]*(cfg["camera"]["cam_v_boost"].as<float>()-1)+1)*cam["radius"]*cam["cam_v"];
	if(key["mov_left"]){
		cam["ctr_x"] -= cam_vel*cos(M_PI*cam["theta"]/180.0);
		cam["ctr_y"] += cam_vel*sin(M_PI*cam["theta"]/180.0);
	}
	if(key["mov_right"]){
		cam["ctr_x"] += cam_vel*cos(M_PI*cam["theta"]/180.0);
		cam["ctr_y"] -= cam_vel*sin(M_PI*cam["theta"]/180.0);
	}
	if(key["mov_back"]){
		cam["ctr_x"] -= cam_vel*sin(M_PI*cam["theta"]/180.0);
		cam["ctr_y"] -= cam_vel*cos(M_PI*cam["theta"]/180.0);
	}
	if(key["mov_forth"]){
		cam["ctr_x"] += cam_vel*sin(M_PI*cam["theta"]/180.0);
		cam["ctr_y"] += cam_vel*cos(M_PI*cam["theta"]/180.0);
	}
	if(key["mov_up"])
		cam["ctr_z"] -= cam_vel;
	if(key["mov_down"])
		cam["ctr_z"] += cam_vel;
}

void keyModifiers(unsigned char k, int mod){
	if(mod == cfg["hotkeys"]["fast_cam"].as<int>())
		key["fast_cam"] = true;
	else
		key["fast_cam"] = false;
}

bool checkKey(unsigned char k, char* key){
	std::vector<char> vi = cfg["hotkeys"][key].as<std::vector<char>>();
	return std::find(vi.begin(),vi.end(),k) != vi.end();
}
 
void keyPressed(unsigned char k, int x, int y){
	if(k == cfg["hotkeys"]["exit"].as<int>())
		exit(0);
	else if(checkKey(k,"plot_axis"))
		view["plot_axis"] = !view["plot_axis"];
	else if(checkKey(k,"reset_view_pos")){
		cam["ctr_x"] = pc.ctroid.x;
		cam["ctr_y"] = pc.ctroid.y;
		cam["ctr_z"] = pc.ctroid.z;
	}
	else if(checkKey(k,"render_p")){
		view["viz_mode"]++;
		if(view["viz_mode"] >= cfg["viz"]["viz_modes"].size())
			view["viz_mode"] = cfg["viz"]["viz_modes"].size()-1;
	}
	else if(checkKey(k,"render_m")){
		view["viz_mode"]--;
		if(view["viz_mode"] < 0)
			view["viz_mode"] = 0;
	}
	else if(checkKey(k,"inc_point"))
		view["point_size"]++;
	else if(checkKey(k,"dec_point")){
		view["point_size"]--;
		if(view["point_size"] < 1)
			view["point_size"] = 1;
	}
	else if(checkKey(k,"zoom_in"))
		key["zoom_in"] = true;
	else if(checkKey(k,"zoom_out"))
		key["zoom_out"] = true;
	else if(checkKey(k,"mov_left"))
		key["mov_left"] = true;
	else if(checkKey(k,"mov_right"))
		key["mov_right"] = true;
	else if(checkKey(k,"mov_back"))
		key["mov_back"] = true;
	else if(checkKey(k,"mov_forth"))
		key["mov_forth"] = true;
	else if(checkKey(k,"mov_down"))
		key["mov_down"] = true;
	else if(checkKey(k,"mov_up"))
		key["mov_up"] = true;
	keyModifiers(k,glutGetModifiers());
}

void keyReleased(unsigned char k, int x, int y){
	if(checkKey(k,"zoom_in"))
		key["zoom_in"] = false;
	else if(checkKey(k,"zoom_out"))
		key["zoom_out"] = false;
	else if(checkKey(k,"mov_left"))
		key["mov_left"] = false;
	else if(checkKey(k,"mov_right"))
		key["mov_right"] = false;
	else if(checkKey(k,"mov_back"))
		key["mov_back"] = false;
	if(checkKey(k,"mov_forth"))
		key["mov_forth"] = false;
	else if(checkKey(k,"mov_down"))
		key["mov_down"] = false;
	else if(checkKey(k,"mov_up"))
		key["mov_up"] = false;
	keyModifiers(k,glutGetModifiers());
}

