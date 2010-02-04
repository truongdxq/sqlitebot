//#include <vector>
//#include <BWTA.h>
//#include <BWTA/Chokepoint.h>
//#include <BWTA/Polygon.h>
//#include <BWTA/Region.h>
//#include <BWTA/BaseLocation.h>
//#include <BWTA/RectangleArray.h>

#include "BasicAIModule.h"
using namespace BWAPI;

/*#include <WorkerManager.h>
#include "Util.h"
#include <algorithm>

using namespace std;
using namespace Util;
WorkerManager::WorkerManager(Arbitrator::Arbitrator<Unit*,double>* arbitrator)
{
  this->arbitrator        = arbitrator;
  this->baseManager       = NULL;
  this->buildOrderManager = NULL;
  this->lastSCVBalance    = 0;
  this->WorkersPerGas     = 3;
  this->mineralRate       = 0;
  this->gasRate           = 0;
  this->autoBuild         = false;
  this->autoBuildPriority = 80;
}
*/



//    int SizeX = 3;
//    int SizeY = 5;
//    std::vector<std::vector<int> > my_2d_int_array(SizeX, std::vector<int>(SizeY));

//int sc_group[30][3];
//int sc_group[2][2]={{1,2},{3,4}};

bool enableStart = true;

//int countTroop = 0;
//int attackStrength = 5;
int scoutStrength = 60;

Unit* myBase=NULL;
int baseRadius = 2000; //default air dist 1200 - ground distance = 2000

//Unit* currentTarget; //unit disappears/unavailable onHide
Position currentTargetPosition[2];
//Position currentHomeTargetPosition;

int groupTargetDistance = 100; //JTC FIX? was 150
int groupCount[2];
int groupCountIdle[2];

int groupCountMarine[2];
int groupCountMedic[2];
int groupCountSV[2];

//int homeTroops = 0;
Unit* hurt[2];

Unit* closestTroopTarget = NULL;

Unit* closestGeyser=NULL;
Unit* closestMineral=NULL;
Unit* scoutWorker=NULL;

//int scoutCount = 1;

//BWAPI::TilePosition tile1;
//BWAPI::TilePosition tile2;
//BWTA::BaseLocation* base1;

//BWAPI::Position mob;

void BasicAIModule::onStart()
{

  this->showManagerAssignments=false;
  if (Broodwar->isReplay()) return;
  // Enable some cheat flags
  Broodwar->enableFlag(Flag::UserInput);
  //Broodwar->enableFlag(Flag::CompleteMapInformation);
  BWTA::readMap();
  BWTA::analyze();
  this->analyzed=true;
  this->buildManager       = new BuildManager(&this->arbitrator);
  this->techManager        = new TechManager(&this->arbitrator);
  this->upgradeManager     = new UpgradeManager(&this->arbitrator);
  //this->scoutManager       = new ScoutManager(&this->arbitrator);
  this->workerManager      = new WorkerManager(&this->arbitrator);
  this->buildOrderManager  = new BuildOrderManager(this->buildManager,this->techManager,this->upgradeManager,this->workerManager);
  this->baseManager        = new BaseManager();
  this->supplyManager      = new SupplyManager();
  //this->defenseManager     = new DefenseManager(&this->arbitrator);
  this->informationManager = new InformationManager();
  this->unitGroupManager   = new UnitGroupManager();
  this->enhancedUI         = new EnhancedUI();

  this->supplyManager->setBuildManager(this->buildManager);
  this->supplyManager->setBuildOrderManager(this->buildOrderManager);
  this->techManager->setBuildingPlacer(this->buildManager->getBuildingPlacer());
  this->upgradeManager->setBuildingPlacer(this->buildManager->getBuildingPlacer());
  this->workerManager->setBaseManager(this->baseManager);
  this->workerManager->setBuildOrderManager(this->buildOrderManager);
  this->baseManager->setBuildOrderManager(this->buildOrderManager);

  //scoutManager->setScoutCount(1); //JTC

  BWAPI::Race race = Broodwar->self()->getRace();
  BWAPI::Race enemyRace = Broodwar->enemy()->getRace();
  BWAPI::UnitType workerType=*(race.getWorker());
    double minDist;
  BWTA::BaseLocation* natural=NULL;
  BWTA::BaseLocation* home=BWTA::getStartLocation(Broodwar->self());
  for(std::set<BWTA::BaseLocation*>::const_iterator b=BWTA::getBaseLocations().begin();b!=BWTA::getBaseLocations().end();b++)
  {
    if (*b==home) continue;
    double dist=home->getGroundDistance(*b);
    if (dist>0)
    {
      if (natural==NULL || dist<minDist)
      {
        minDist=dist;
        natural=*b;
      }
    }
  }
  this->buildOrderManager->enableDependencyResolver();
  //make the basic production facility
  if (race == Races::Zerg)
  {
    //send an overlord out if Zerg
    //this->scoutManager->setScoutCount(1);

    //12 hatch
    this->buildOrderManager->build(12,workerType,80);
    this->baseManager->expand(natural,79);
    this->buildOrderManager->build(20,workerType,78);
    this->buildOrderManager->buildAdditional(1,UnitTypes::Zerg_Spawning_Pool,60);
    this->buildOrderManager->buildAdditional(3,UnitTypes::Zerg_Zergling,82);
  }
  else if (race == Races::Terran)
  {
    this->buildOrderManager->build(22,workerType,200); //was 120 priority - online fix?
	this->buildOrderManager->build(5,workerType,74);
	/*
    if (enemyRace == Races::Zerg)
    {
      this->buildOrderManager->buildAdditional(1,UnitTypes::Terran_Barracks,60);
      this->buildOrderManager->buildAdditional(9,UnitTypes::Terran_Marine,45);
      this->buildOrderManager->buildAdditional(1,UnitTypes::Terran_Refinery,42);
      this->buildOrderManager->buildAdditional(1,UnitTypes::Terran_Barracks,40);
      this->buildOrderManager->buildAdditional(1,UnitTypes::Terran_Academy,39);
      this->buildOrderManager->buildAdditional(9,UnitTypes::Terran_Medic,38);
      this->buildOrderManager->research(TechTypes::Stim_Packs,35);
      this->buildOrderManager->research(TechTypes::Tank_Siege_Mode,35);
      this->buildOrderManager->buildAdditional(3,UnitTypes::Terran_Siege_Tank_Tank_Mode,34);
      this->buildOrderManager->buildAdditional(2,UnitTypes::Terran_Science_Vessel,30);
	  this->buildOrderManager->research(TechTypes::Irradiate,30);
      this->buildOrderManager->upgrade(1,UpgradeTypes::Terran_Infantry_Weapons,20);
      this->buildOrderManager->build(3,UnitTypes::Terran_Missile_Turret,13);
      this->buildOrderManager->upgrade(3,UpgradeTypes::Terran_Infantry_Weapons,12);
      this->buildOrderManager->upgrade(3,UpgradeTypes::Terran_Infantry_Armor,12);
      this->buildOrderManager->build(1,UnitTypes::Terran_Engineering_Bay,11);
      this->buildOrderManager->buildAdditional(40,UnitTypes::Terran_Marine,10);
      this->buildOrderManager->build(6,UnitTypes::Terran_Barracks,8);
      this->buildOrderManager->build(2,UnitTypes::Terran_Engineering_Bay,7);
      this->buildOrderManager->buildAdditional(10,UnitTypes::Terran_Siege_Tank_Tank_Mode,5);
    }
    else
    {
	*/
	  if (enableStart) {
	  //medics at 1 to 5 troop ratio
      //this->buildOrderManager->buildAdditional(10,UnitTypes::Terran_Marine,100);
      //this->buildOrderManager->buildAdditional(2,UnitTypes::Terran_Medic,100);
      this->buildOrderManager->buildAdditional(3,UnitTypes::Terran_Medic,119);
	  this->buildOrderManager->buildAdditional(20,UnitTypes::Terran_Marine,118);

      /*this->buildOrderManager->buildAdditional(3,UnitTypes::Terran_Medic,117);
	  this->buildOrderManager->buildAdditional(15,UnitTypes::Terran_Marine,116);
      this->buildOrderManager->buildAdditional(3,UnitTypes::Terran_Medic,115);
	  this->buildOrderManager->buildAdditional(15,UnitTypes::Terran_Marine,114);
      this->buildOrderManager->buildAdditional(3,UnitTypes::Terran_Medic,113);
	  this->buildOrderManager->buildAdditional(15,UnitTypes::Terran_Marine,112);
	  this->buildOrderManager->buildAdditional(3,UnitTypes::Terran_Medic,111);
	  this->buildOrderManager->buildAdditional(15,UnitTypes::Terran_Marine,111);
	  */
	  this->buildOrderManager->buildAdditional(6,UnitTypes::Terran_Medic,73);
      this->buildOrderManager->buildAdditional(22,UnitTypes::Terran_Marine,72);
	  this->buildOrderManager->buildAdditional(6,UnitTypes::Terran_Medic,71);
      this->buildOrderManager->buildAdditional(23,UnitTypes::Terran_Marine,70);

      //this->buildOrderManager->buildAdditional(1,UnitTypes::Terran_Barracks,86);
      this->buildOrderManager->buildAdditional(1,UnitTypes::Terran_Barracks,109);
	  this->buildOrderManager->buildAdditional(2,UnitTypes::Terran_Barracks,75);

      //this->buildOrderManager->buildAdditional(3,UnitTypes::Terran_Barracks,109);
      this->buildOrderManager->buildAdditional(1,UnitTypes::Terran_Academy,109);

	  this->buildOrderManager->buildAdditional(1,UnitTypes::Terran_Science_Facility,85);

	  this->buildOrderManager->buildAdditional(1,UnitTypes::Terran_Control_Tower,74);
	  this->buildOrderManager->buildAdditional(1,UnitTypes::Terran_Armory,74);
	  this->buildOrderManager->upgrade(3,UpgradeTypes::Terran_Ship_Plating,102);      

	  //this->buildOrderManager->buildAdditional(4,UnitTypes::Terran_Missile_Turret,79);  
	  this->buildOrderManager->buildAdditional(3,UnitTypes::Terran_Science_Vessel,74); 

      this->buildOrderManager->upgrade(1,UpgradeTypes::Terran_Infantry_Weapons,106);      
	  this->buildOrderManager->upgrade(3,UpgradeTypes::Terran_Infantry_Weapons,104);
	  this->buildOrderManager->upgrade(1,UpgradeTypes::Terran_Infantry_Armor,105);
	  this->buildOrderManager->upgrade(3,UpgradeTypes::Terran_Infantry_Armor,103);

	  this->buildOrderManager->upgrade(1,UpgradeTypes::U_238_Shells,104);

	  this->buildOrderManager->research(TechTypes::Healing,104);
	  this->buildOrderManager->upgrade(1,UpgradeTypes::Caduceus_Reactor,103);
	  this->buildOrderManager->research(TechTypes::Restoration,102);
	  //this->buildOrderManager->research(TechTypes::Stim_Packs,101);

	  //science vessel - 2 units, research, upgrades
	  //attacks?
	  //attack target if idle? not for now since more vulnerable
	  }
	  else 
	  {
	  this->buildOrderManager->buildAdditional(12,UnitTypes::Terran_Medic,73);
      this->buildOrderManager->buildAdditional(30,UnitTypes::Terran_Marine,73);
	  this->buildOrderManager->buildAdditional(3,UnitTypes::Terran_Science_Vessel,74); 
	  this->buildOrderManager->research(TechTypes::Restoration,102);
	  }

	  /*
      this->buildOrderManager->buildAdditional(2,BWAPI::UnitTypes::Terran_Machine_Shop,70);
      this->buildOrderManager->buildAdditional(3,BWAPI::UnitTypes::Terran_Factory,60);
	  //this->buildOrderManager->research(TechTypes::Spider_Mines,55);
      //this->buildOrderManager->research(TechTypes::Tank_Siege_Mode,55);
      this->buildOrderManager->buildAdditional(20,BWAPI::UnitTypes::Terran_Vulture,40);
      //this->buildOrderManager->buildAdditional(20,BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode,40);
	  this->buildOrderManager->upgrade(3,UpgradeTypes::Ion_Thrusters,40);
      this->buildOrderManager->upgrade(3,UpgradeTypes::Terran_Vehicle_Weapons,40);
	  this->buildOrderManager->upgrade(3,UpgradeTypes::Terran_Vehicle_Plating,40);
 */

	//}
  }
  else if (race == Races::Protoss)
  {
    this->buildOrderManager->build(20,workerType,80);
    this->buildOrderManager->buildAdditional(10,UnitTypes::Protoss_Dragoon,70);
    this->buildOrderManager->buildAdditional(10,UnitTypes::Protoss_Zealot,70);
    this->buildOrderManager->upgrade(1,UpgradeTypes::Singularity_Charge,61);
    this->buildOrderManager->buildAdditional(20,UnitTypes::Protoss_Carrier,60);
  }
  this->workerManager->enableAutoBuild();
  this->workerManager->setAutoBuildPriority(40);
 
}


void BasicAIModule::onFrame()
{
  if (Broodwar->isReplay()) return;
  //JTC if (!this->analyzed) return;
  //Broodwar->sendText("Update");
  this->buildManager->update();
  this->buildOrderManager->update();
  this->baseManager->update();
  //JTC this->workerManager->update(); //don't mess with my worker handling
  this->techManager->update();
  this->upgradeManager->update();
  this->supplyManager->update();
  //JTC this->scoutManager->update();
  //JTC this->defenseManager->update();  //don't send marines to chokepoints and bottle routes up
  this->arbitrator.update();

  this->enhancedUI->update();

  //JTCif (Broodwar->getFrameCount()>24*50)
    //scoutManager->setScoutCount(1);

  if (enableScout) 
  {
	//Broodwar->sendText("enableScout");
	//if (scoutWorker==NULL) { this->buildOrderManager->build(1,BWAPI::UnitTypes::Terran_SCV,120); scoutManager->setScoutCount(0); }
	//else { this->scoutManager->update(); scoutManager->setScoutCount(1); }
  }

  //if (Broodwar->getFrameCount()>24) { this->scoutManager->update(); scoutManager->setScoutCount(1); }
  //if (Broodwar->getFrameCount()>700 && Broodwar->getFrameCount() < 1400) { this->scoutManager->update(); scoutManager->setScoutCount(0); }
  //if (Broodwar->getFrameCount()>1400 && Broodwar->getFrameCount() < 2100) { this->scoutManager->update(); scoutManager->setScoutCount(1); }


  std::set<Unit*> units=Broodwar->self()->getUnits();
  if (this->showManagerAssignments)
  {
    for(std::set<Unit*>::iterator i=units.begin();i!=units.end();i++)
    {
      if (this->arbitrator.hasBid(*i))
      {
        int x=(*i)->getPosition().x();
        int y=(*i)->getPosition().y();
        std::list< std::pair< Arbitrator::Controller<BWAPI::Unit*,double>*, double> > bids=this->arbitrator.getAllBidders(*i);
        int y_off=0;
        bool first = false;
        const char activeColor = '\x07', inactiveColor = '\x16';
        char color = activeColor;
        for(std::list< std::pair< Arbitrator::Controller<BWAPI::Unit*,double>*, double> >::iterator j=bids.begin();j!=bids.end();j++)
        {
          Broodwar->drawText(CoordinateType::Map,x,y+y_off,"%c%s: %d",color,j->first->getShortName().c_str(),(int)j->second);
          y_off+=15;
          color = inactiveColor;
        }
      }
    }
  }

  UnitGroup myPylonsAndGateways = SelectAll()(Pylon,Gateway)(HitPoints,"<=",200);
  for each(Unit* u in myPylonsAndGateways)
  {
    Broodwar->drawCircleMap(u->getPosition().x(),u->getPosition().y(),20,Colors::Red);
  }
  
  if ((Broodwar->getFrameCount() % 61) == 0) { checkIdle(); }
  if ((Broodwar->getFrameCount() % 30) == 0) { groupMove(); }

  //check resource at beginning initially, don't wait
  if (Broodwar->getFrameCount() < 10000)  //FIX - workers idle/conflicted with buildmanager? 
  {
	if ((Broodwar->getFrameCount() % 30) == 0) { checkResources(); }
  }
  else if ((Broodwar->getFrameCount() % 501) == 0) { checkResources(); }

}

void BasicAIModule::onUnitDestroy(BWAPI::Unit* unit)
{
  this->arbitrator.onRemoveObject(unit);
  this->buildManager->onRemoveUnit(unit);
  this->techManager->onRemoveUnit(unit);
  this->upgradeManager->onRemoveUnit(unit);
  this->workerManager->onRemoveUnit(unit);
  //this->scoutManager->onRemoveUnit(unit);
  //this->defenseManager->onRemoveUnit(unit);
  this->informationManager->onUnitDestroy(unit);

  Broodwar->sendText("UnitDestroy:%s",unit->getType().getName().c_str());
  //replace our destroyed units
  if (unit->getPlayer()->getID() == Broodwar->self()->getID()) {

  if (!strcmp(unit->getType().getName().c_str(),"Terran Vulture")) 
    {
    this->buildOrderManager->buildAdditional(1,BWAPI::UnitTypes::Terran_Vulture,100);
    } 
  if (!strcmp(unit->getType().getName().c_str(),"Terran Marine")) 
    {
    this->buildOrderManager->buildAdditional(1,BWAPI::UnitTypes::Terran_Marine,100);
    } 
  if (!strcmp(unit->getType().getName().c_str(),"Terran Medic")) 
    {
    this->buildOrderManager->buildAdditional(1,BWAPI::UnitTypes::Terran_Medic,101);
    } 
  if (!strcmp(unit->getType().getName().c_str(),"Terran SCV")) 
    {
	//JTC FIX scoutManager->setScoutCount(0);
    this->buildOrderManager->buildAdditional(1,BWAPI::UnitTypes::Terran_SCV,102);
    }
  if (!strcmp(unit->getType().getName().c_str(),"Terran Science Vessel")) 
    {
		this->buildOrderManager->buildAdditional(1,BWAPI::UnitTypes::Terran_Science_Vessel,102);
    }

  if (!strcmp(unit->getType().getName().c_str(),"Terran Barracks")) 
    {
		this->buildOrderManager->buildAdditional(1,BWAPI::UnitTypes::Terran_Barracks,102);
    } 
  if (!strcmp(unit->getType().getName().c_str(),"Terran Command Center")) 
    {
		this->buildOrderManager->buildAdditional(1,BWAPI::UnitTypes::Terran_Command_Center,102);
    } 

	for(std::list<GroupData*>::const_iterator i=mainGroup.begin();i!=mainGroup.end();i++)
	  {
	  //if ( ((*i)->unitPointer == unit) && (*i)->move )
	  if ((*i)->unitPointer == unit)
		  {
		  //(*i)->move = false;
		  mainGroup.erase(i);  
		  break;
	      }
	  }

	if (unit == scoutWorker) { scoutWorker=NULL; } //JTC - FIX

  }
}

void BasicAIModule::onUnitShow(BWAPI::Unit* unit)
{
  this->informationManager->onUnitShow(unit);
  this->unitGroupManager->onUnitShow(unit);

  if (!strcmp(unit->getType().getName().c_str(),"Resource Mineral Field")
	  || !strcmp(unit->getType().getName().c_str(),"Resource Vespene Geyser")
	  )
  { checkResources(); return; } //make sure assign closest while visible/show ?

  if (unit->getType().isNeutral()
	  || !strcmp(unit->getType().getName().c_str(),"Special Power Generator")
	  )
  { return; }

//------------------------
  if (unit->getPlayer()->getID() == Broodwar->self()->getID()) {

  if (!strcmp(unit->getType().getName().c_str(),"Terran Marine")
	  || !strcmp(unit->getType().getName().c_str(),"Terran Medic")
	  || !strcmp(unit->getType().getName().c_str(),"Terran Science Vessel")
	  ) 
    {
    groupAdd(unit);
    } 

  if (scoutWorker==NULL
	  && enableScout
	  && !strcmp(unit->getType().getName().c_str(),"Terran SCV")
	  ) 
    {
    scoutWorker=unit;
    } 
  }

//------------------------  

  int thisTime = Broodwar->getFrameCount();
  Broodwar->sendText("%s[%x]spotted(%d,%d):time:%d",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y(),thisTime);

  static int attackTime;

  if (thisTime < attackTime+24*5) { return; }
  
  //attackTime = thisTime;
  //Broodwar->sendText("Attack!");

  //if (!((*i)->getPlayer()->getID()==Broodwar->self()->getID()))
  
  //Broodwar->sendText("attackTime:%d:Neutral:%d",attackTime,unit->getType().isNeutral());

  //don't attack if low strength and seen enemy far away
  if (myBase==NULL) { return; }
  //if (countTroop < attackStrength && ((unit)->getDistance(myBase) > baseRadius)) { return; }

  //attack if not self
  if (unit->getPlayer()->getID() != Broodwar->self()->getID()
	  //&& !(unit->getType().isNeutral())
	  //&& strcmp(unit->getType().getName().c_str(),"Special Power Generator")
	  ) 
    {
    Broodwar->sendText("Attack!");
    attackTime = thisTime;

	BWAPI::Position target;
	target.x() = unit->getPosition().x();
	target.y() = unit->getPosition().y();

	if (!strcmp(unit->getType().getName().c_str(),"Unknown")) { return; }
	//double distBase = BWTA::getGroundDistance(unit->getTilePosition(),myBase->getTilePosition());
	double distBase = BWTA::getGroundDistance(unit->getPosition(),myBase->getPosition());

	//double dist = unit->getDistance(myBase);
	if (distBase < baseRadius && distBase != -1) {
		currentTargetPosition[0] = target;
		return; }
	else { currentTargetPosition[1] = target; }

	//Unit* closestTarget=NULL;

	std::set<Unit*> units=Broodwar->self()->getUnits();

	for(std::set<Unit*>::iterator i=units.begin();i!=units.end();i++)
	  {
	  //ignore others units
	 if ((*i)->getPlayer()->getID() != Broodwar->self()->getID()) { continue; }

      //don't send workers,medics,flyers to attack
	  if (!((*i)->getType().isWorker()) &&
		  (strcmp((*i)->getType().getName().c_str(),"Terran Medic"))  &&
		  !(*i)->getType().isFlyer()
		  && !(*i)->getType().isBuilding()
		  )
	    {
	    //(*i)->attackMove(target);
		groupAdd(*i);

        //if (closestTarget==NULL || (*i)->getDistance(target)<(*i)->getDistance(closestTarget))
			//{ closestTarget=*i; }
	    }

	}

	for(std::set<Unit*>::iterator i=units.begin();i!=units.end();i++)
	  {
	  //ignore others units
	 if ((*i)->getPlayer()->getID() != Broodwar->self()->getID()) { continue; }

	  //idle medics,sv can join attack
	  if ( !strcmp((*i)->getType().getName().c_str(),"Terran Medic")
		  && closestTroopTarget!=NULL
		  //&& (*i)->isIdle()
		  ) {
		//(*i)->attackMove(target);
		groupAdd(*i);
		//tried closest but maybe coming around corner problems
		//(*i)->rightClick(closestTroopTarget->getPosition());
	  }

	  if ( !strcmp((*i)->getType().getName().c_str(),"Terran Science Vessel") 
		  && closestTroopTarget!=NULL
		  ) {
	    groupAdd(*i);
		//(*i)->rightClick(closestTroopTarget->getPosition());
	  }
	}

  }
}

void BasicAIModule::onUnitHide(BWAPI::Unit* unit)
{
  this->informationManager->onUnitHide(unit);
  this->unitGroupManager->onUnitHide(unit);
}

void BasicAIModule::onUnitMorph(BWAPI::Unit* unit)
{
  this->unitGroupManager->onUnitMorph(unit);
}
void BasicAIModule::onUnitRenegade(BWAPI::Unit* unit)
{
  this->unitGroupManager->onUnitRenegade(unit);
}

bool BasicAIModule::onSendText(std::string text)
{
  UnitType type=UnitTypes::getUnitType(text);
  if (text=="debug")
  {
    this->showManagerAssignments=true;
    this->buildOrderManager->enableDebugMode();
    //this->scoutManager->enableDebugMode();
    return true;
  }
  if (text=="expand")
  {
    this->baseManager->expand();
  }
  if (type!=UnitTypes::Unknown)
  {
    this->buildOrderManager->buildAdditional(1,type,300);
  }
  else
  {
    TechType type=TechTypes::getTechType(text);
    if (type!=TechTypes::Unknown)
    {
      this->techManager->research(type);
    }
    else
    {
      UpgradeType type=UpgradeTypes::getUpgradeType(text);
      if (type!=UpgradeTypes::Unknown)
      {
        this->upgradeManager->upgrade(type);
      }
      else
        Broodwar->printf("You typed '%s'!",text.c_str());
    }
  }
  return true;
}

void BasicAIModule::checkIdle()
{
//Broodwar->sendText("ciS");
	Broodwar->sendText("Frame:%d",Broodwar->getFrameCount());

	int workerCount = 0;
	int idleWorkers = 0;
	int gasWorkers = 0;
	int mineralWorkers = 0;
	//countTroop = 0;
	//int totalX = 0;
	//int totalY = 0;
    //int totalIdle = 0;
	//bool idleMob = false;

	//Unit* hurt=NULL;
    
	std::set<Unit*> repairUnits;

//check our list of repairs already in progress to see if finished and remove from list

	/*
  for (std::set<Unit*>::const_iterator u=repairUnits.begin(); u!=repairUnits.end(); u++) {
	  if ((*u)->getType().getID()==(*m)->getType().getID()) { repairUnits.erase(
	  {

	  }
  }*/

	//workerUnit
	//vector<int> dyarray;
	//arrayRepair[1][1] = 55;


	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
    {
	  //ignore others units
	 if ((*i)->getPlayer()->getID() != Broodwar->self()->getID()) { continue; }

		if ((*i)->getType().isWorker()) {
		  workerCount++;
		  if ((*i)->isGatheringMinerals()) { mineralWorkers++; }
		  if ((*i)->isGatheringGas()) { gasWorkers++; }
		  if ((*i)->isIdle()) { idleWorkers++; }
		}
		//don't count workers,buildings with troops
		//else if (!(*i)->getType().isBuilding() && !(*i)->getType().isFlyer()) {
		/*
		else if (!(*i)->getType().isBuilding()) {
			countTroop++;

			if ((*i)->isIdle()) { totalIdle++; }

			//totalX += (*i)->getPosition().x();
			//totalY += (*i)->getPosition().y();

			//if ((*i)->getHitPoints() < (*i)->getInitialHitPoints()) 
		}
		*/
		/*
			if ((*i)->getHitPoints() < (*i)->getType().maxHitPoints() ||
				(*i)->isBlind()				
				) 
				hurt = *i;
		}

	    if (!strcmp((*i)->getType().getName().c_str(),"Terran Medic") && hurt != NULL) {
			if ((hurt)->isBlind()) { (*i)->useTech(BWAPI::TechTypes::Restoration,hurt); }
			else { if (!(*i)->getType().isFlyer()) { (*i)->rightClick(hurt);} }
	    }
		if (!strcmp((*i)->getType().getName().c_str(),"Terran Science Vessel") && hurt != NULL) {
		  //try follow as right-click brings too much air counter-attack
	      (*i)->rightClick(hurt);
	    }
		*/

		//which unit base? more than one?
		if (!strcmp((*i)->getType().getName().c_str(),"Terran Command Center"))
		{
			myBase = *i;
		}

	}
	//JTC - FIX bug divide by zero troop at game start
	//int avgX = totalX/countTroop;
	//int avgY = totalY/countTroop;  

	//mob.x() = avgX;
	//mob.y() = avgY;

	//if (countTroop > scoutStrength) { enableScout = true; }

	//if (totalIdle == countTroop) { idleMob = true; }

	Broodwar->sendText("w(%d)%d/%d:%d",workerCount,mineralWorkers,gasWorkers,idleWorkers);

    //send each worker to the mineral field that is closest to it
    for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
    {
	  if (scoutWorker!=NULL && *i==scoutWorker) { continue; }
      if ((*i)->getType().isWorker() && (*i)->isIdle())
      {
		
		//double mineralRate = this->workerManager->getMineralRate();
		//double gasRate = this->workerManager->getGasRate();

		 // Broodwar->sendText("idle:gasWorkers:%d:mineral:%d",gasWorkers,mineralWorkers);
		//gas
		if (gasWorkers < 3
			&& closestGeyser!=NULL //null check
			&& closestGeyser->getType().isRefinery()
			&& closestGeyser->isCompleted()
			)
		{
  		  //Broodwar->sendText("debug0");
		  (*i)->rightClick(closestGeyser); 
		}
		//minerals
		else
		{
		  if (closestMineral!=NULL) { (*i)->rightClick(closestMineral); }
		}
	  } //idle workers

//----scout/repairBegin
	  //if ((*i)->getType().isWorker() && !(*i)->isRepairing()) {
	  if ((*i)->getType().isWorker()) {

//scout
/*	if (enableScout)
	{
		if (scoutWorker==NULL) {
			scoutWorker=*i;
		}
		if (*i == scoutWorker)
		{
			std::pair<std::list<BWTA::BaseLocation*>, double> getBestPathHelper(std::set<BWTA::BaseLocation* > baseLocations)
			for(std::set<BWTA::BaseLocation*>::iterator b=baseLocations.begin();b!=baseLocations.end();b++)
			{
			  (*i)->rightClick((*b)->getPosition());
			}
		  	continue;
		}
	}
*/


//repair
  	  //repair trumps others
          Unit* closestRepair=NULL;
   	  	  for(std::set<Unit*>::const_iterator m=Broodwar->self()->getUnits().begin();m!=Broodwar->self()->getUnits().end();m++)
    {

	  //ignore others units
	 if ((*m)->getPlayer()->getID() != Broodwar->self()->getID()) { continue; }

//check our list of repairs already in progress so we don't repeat assign to others
  bool alreadyRepairing = false;
  for (std::set<Unit*>::const_iterator u=repairUnits.begin(); u!=repairUnits.end(); u++) {
	  //FIX - ? getID broke? need group/pointer ?
	  if ((*u)->getType().getID()==(*m)->getType().getID()) {
		  alreadyRepairing = true;
		  break;
	  }
  }
			  
  if (alreadyRepairing) { continue; } //break; ?

			  //for(std::set<Unit*>::iterator m=Broodwar->getAllUnits().begin();m!=Broodwar->getAllUnits().end();m++)
		
			  //Broodwar->sendText("rtype:%s:%d:%d",(*m)->getType().getName().c_str(),(*m)->getHitPoints(),(*m)->getInitialHitPoints());
			  //Broodwar->sendText("m:%d",(*m)->getType().isRefinery());
			  //repair buildings,detectors(sv,missilesilo),workers
			  if (((*m)->getType().isBuilding() || (*m)->getType().isDetector() || (*m)->getType().isWorker())
				  //(*m)->getPlayer()==Broodwar->self() &&
				  //((*m)->getHitPoints() < (*m)->getInitialHitPoints())
				  && ((*m)->getHitPoints() < (*m)->getType().maxHitPoints())
				  ) {

			//Broodwar->sendText("repair:%s",(*m)->getType().getName().c_str());
            if (closestRepair==NULL || (*i)->getDistance(*m)<(*i)->getDistance(closestRepair))
			{ closestRepair=*m; }
		  }
		  }  //for all our units
			if (closestRepair!=NULL) { (*i)->repair(closestRepair); repairUnits.insert(closestRepair); }
	  
	  } //if worker
//----scout/repairEnd

	} // for all self units

    //base defense

	
	int enemyCount = 0;
    for(std::set<Unit*>::iterator m=Broodwar->getAllUnits().begin();m!=Broodwar->getAllUnits().end();m++)
	{
		//Broodwar->sendText("BA:%.2f:%s",(*m)->getDistance(myBase),(*m)->getType().getName().c_str());

		//if !worker,!self,!neutral,!power and enemy and close to base
		if (
			
			//(!(*i)->getType().isWorker())
			 //&& strcmp((*i)->getType().getName().c_str(),"Terran Medic") 
			 //&& strcmp((*i)->getType().getName().c_str(),"Terran Science Vessel")

		     ((*m)->getPlayer()->getID() != Broodwar->self()->getID())
			 && !((*m)->getType().isNeutral())
			 && strcmp((*m)->getType().getName().c_str(),"Special Power Generator")
			 && strcmp((*m)->getType().getName().c_str(),"Vulture Spider Mine")
			 && strcmp((*m)->getType().getName().c_str(),"Unknown")
			 //&& !(*i)->getType().isBuilding()
			)
		{
			enemyCount++;
			//Broodwar->sendText("BAC:%s",(*m)->getType().getName().c_str());

			//if within baseRadius, always attack - otherwise only attack if idle(not already attacking)
			//double distBase = (*m)->getDistance(myBase);
			//double distBase = BWTA::getGroundDistance((*m)->getTilePosition(),myBase->getTilePosition());
			double distBase = BWTA::getGroundDistance((*m)->getPosition(),myBase->getPosition());

			//if ((*m)->getDistance(myBase) < baseRadius)
			if (distBase < baseRadius && distBase != -1)
			  {
			  Broodwar->sendText("BAC:%.2f:%s",distBase,(*m)->getType().getName().c_str());
			  //(*i)->attackMove((*m)->getPosition());
			  currentTargetPosition[0] = (*m)->getPosition();
			  break; //just focus on first unit that is listed
			  }
			//else if ((*i)->isIdle()) { (*i)->attackMove((*m)->getPosition()); }
			else if (currentTargetPosition[1].x()==NULL) { currentTargetPosition[1] = (*m)->getPosition(); }
		} //if enemy
	} //all units

	Broodwar->sendText("e:%d",enemyCount);

	if (enemyCount == 0  && mainGroup.size() > 55) { enableScout = true; }
	else { enableScout = false; }


//Broodwar->sendText("ciE");
}

void BasicAIModule::groupAdd(BWAPI::Unit* unit)
{
//Broodwar->sendText("gaS");
	//Broodwar->sendText("trygroupAdd:%d",unit->getType().getID());

//don't duplicate insert existing unit to group
	for(std::list<GroupData*>::const_iterator i=mainGroup.begin();i!=mainGroup.end();i++)
{
	//Broodwar->sendText("add:%d:%d",(*i)->getType().getID(),unit->getType().getID());
	if ((*i)->unitPointer == unit) { return; } 
}

//if (mainGroup.size() > 5) { return; }

//-----------------
int groupID;

if (!strcmp(unit->getType().getName().c_str(),"Terran Medic"))
{
	if (groupCountMedic[0] < 4) { groupID = 0; }
	else { groupID = 1; }
	groupCountMedic[groupID]++;
}

if (!strcmp(unit->getType().getName().c_str(),"Terran Marine"))
{
	if (groupCountMarine[0] < 20) { groupID = 0; }
	else { groupID = 1; }
	groupCountMarine[groupID]++;
}

if (!strcmp(unit->getType().getName().c_str(),"Terran Science Vessel"))
{
	if (groupCountSV[0] < 1) { groupID = 0; }
	else { groupID = 1; }
	groupCountSV[groupID]++;
}

groupCount[groupID]++;
//-----------------

//Broodwar->sendText("groupAdd:%d",unit->getType().getID());

GroupData* thisGroupData = new GroupData();
//(*thisGroupData).move = true;
(*thisGroupData).unitID = unit->getType().getID();
(*thisGroupData).unitPointer = unit;
(*thisGroupData).groupID = groupID;
mainGroup.push_back(thisGroupData);

//Broodwar->sendText("gaE");
}

//---------------------------------------------//
void BasicAIModule::groupMove()
{
//Broodwar->sendText("gmS");

if (currentTargetPosition[0].x()!=NULL) { Broodwar->drawCircle(CoordinateType::Map,currentTargetPosition[0].x(),currentTargetPosition[0].y(),10,Colors::Red); }
if (currentTargetPosition[1].x()!=NULL) { Broodwar->drawCircle(CoordinateType::Map,currentTargetPosition[1].x(),currentTargetPosition[1].y(),20,Colors::Red); }


int stagedDistance[2];
int stagedTroops[2];
int numStagedAttack[2];
int battleDistance[2];  
int numBattleAttack[2];
int battleTroops[2];
int targetDistance[2];
int targetTroops[2];

//----
stagedTroops[0] = 0;
stagedDistance[0] = 0;
numStagedAttack[0] = 0;
battleDistance[0] = 0;  //1200-900 = 300 area gap for waves
numBattleAttack[0] = 0;
targetDistance[0] = 10;

groupCount[0] = 0;
groupCountMarine[0] = 0;
groupCountMedic[0] = 0;
groupCountSV[0] = 0;
groupCountIdle[0] = 0;
battleTroops[0] = 0;
targetTroops[0] = 0;

//----
stagedTroops[1] = 0;
stagedDistance[1] = 1200;
numStagedAttack[1] = 28;
battleDistance[1] = 900;  
numBattleAttack[1] = 5;
targetDistance[1] = 10;

groupCount[1] = 0;
groupCountMarine[1] = 0;
groupCountMedic[1] = 0;
groupCountSV[1] = 0;
groupCountIdle[1] = 0;
battleTroops[1] = 0;
targetTroops[1] = 0;

hurt[0]=NULL;
hurt[1]=NULL;

//----
for (int groupID = 0; groupID <= 1; groupID++ ) {
for(std::list<GroupData*>::const_iterator i=mainGroup.begin();i!=mainGroup.end();i++)
{
  if ((*i)->groupID != groupID) { continue; }

  //if ((*i)->groupID == 0) { homeTroops++; }

  Unit* u;
  u = (*i)->unitPointer;

  double dist = BWTA::getGroundDistance(u->getPosition(),currentTargetPosition[groupID]);
  (*i)->currentTargetDistance = dist;
  if (dist != -1 && dist < stagedDistance[groupID]) { stagedTroops[groupID]++; }
  if (dist != -1 && dist < battleDistance[groupID]) { battleTroops[groupID]++; }
  if (dist != -1 && dist < targetDistance[groupID]) { targetTroops[groupID]++; }
  groupCount[groupID]++;

  //this should identify at least one hurt/group in full pass if exists
  if (u->getHitPoints() < u->getType().maxHitPoints()
	|| u->isBlind()				
	) 
  { hurt[groupID] = u; }
  //{ hurt[groupID] = u; if (u->isBlind()) {Broodwar->sendText("blind");} }

  if (u->isIdle()) { groupCountIdle[groupID]++; }

  if (!strcmp(u->getType().getName().c_str(),"Terran Marine")) { groupCountMarine[groupID]++; }
  if (!strcmp(u->getType().getName().c_str(),"Terran Medic")) { groupCountMedic[groupID]++; }
  if (!strcmp(u->getType().getName().c_str(),"Terran Science Vessel")) { groupCountSV[groupID]++; }
}

Broodwar->sendText("g%dMove(%d)%d(%d/%d/%d)%d:%d:%d",groupID,mainGroup.size(),groupCount[groupID],groupCountMarine[groupID],groupCountMedic[groupID],groupCountSV[groupID],stagedTroops[groupID],battleTroops[groupID],groupCountIdle[groupID]);

//if we've gotten to or destroyed our target, then forget it - FIX/remove targetTroops?
//FIX bug - self-defeating loop -
//if (groupCountIdle[groupID] > 10) { currentTargetPosition[groupID].x()=NULL; currentTargetPosition[groupID].y()=NULL;}

} //for groupID

//---------------------------------------------//

for (int groupID = 0; groupID <= 1; groupID++ ) {

double closestDistance = 30000; //init
	
for(std::list<GroupData*>::const_iterator i=mainGroup.begin();i!=mainGroup.end();i++)
{
	//Broodwar->sendText("groupType:%s",(*i)->unitPointer->getType().getName().c_str());
	//if (!(*i)->move) { continue; }
	if ((*i)->groupID != groupID) { continue; }
	//Broodwar->sendText("move:%d:%.1f:%d:%d",(*i)->getType().getID(),(*i)->getDistance(currentTargetPosition),currentTargetPosition.x(),currentTargetPosition.y());

	    Unit* u;
		u = (*i)->unitPointer;
/*
      //tried this, but medic get in the way of advancement, especially ramp chokepoints
	  if ( !strcmp((*i)->unitPointer->getType().getName().c_str(),"Terran Medic")
		  && closestTroopTarget!=NULL
		  //&& (*i)->isIdle()
		  ) {
		//(*i)->attackMove(target);
		//tried closest but maybe coming around corner problems
		(*i)->unitPointer->rightClick(closestTroopTarget->getPosition());
	  }
	  else { u->attackMove(currentTargetPosition); }
*/

double dist = (*i)->currentTargetDistance;

if ( closestTroopTarget==NULL || (dist < closestDistance && dist != -1) )
  { closestTroopTarget=u; closestDistance = dist; }

if ( currentTargetPosition[groupID].x()!=NULL
	&&dist > groupTargetDistance //how close to target need be? density of mob
	//&& dist != -1 JTC FIX?
	&& (!strcmp(u->getType().getName().c_str(),"Terran Marine")
	   || (!strcmp(u->getType().getName().c_str(),"Terran Medic") && u->isIdle()) //get medics out of way
	   || (!strcmp(u->getType().getName().c_str(),"Terran Science Vessel") && u->isIdle())  //SV gets shot at by direct air, needs to travel safer troop ground path
	   )
	) 
	{ u->attackMove(currentTargetPosition[groupID]); }

//stop if medic, but continue if someone hurt

if (battleTroops[groupID] < numBattleAttack[groupID]
	&& dist < stagedDistance[groupID]
	&& stagedTroops[groupID] < numStagedAttack[groupID]
	&& dist != -1
	)
  { u->stop(); }

//SV heal priority over marines
if (!strcmp(u->getType().getName().c_str(),"Terran Medic") && hurt[groupID] != NULL) {
	//if ((hurt[groupID])->isBlind()) { u->rightClick(hurt[groupID]); u->useTech(BWAPI::TechTypes::Restoration,hurt[groupID]); Broodwar->sendText("BlindHeal"); }
	if ((hurt[groupID])->isBlind()) { u->useTech(BWAPI::TechTypes::Restoration,hurt[groupID]); Broodwar->sendText("BlindHeal"); }
	else { if (!u->getType().isFlyer()) { u->rightClick(hurt[groupID]);} }
}
if (!strcmp(u->getType().getName().c_str(),"Terran Science Vessel")
	&& hurt[groupID] != NULL
	&& !strcmp(hurt[groupID]->getType().getName().c_str(),"Terran Marine") //only follow hurt marines
	) {
  //try follow as right-click brings too much air counter-attack
  u->rightClick(hurt[groupID]);
}

}  //for i

if (closestTroopTarget!= NULL) { (closestTroopTarget)->stop(); }

if (hurt[0]!=NULL) { Broodwar->drawCircle(CoordinateType::Map,hurt[0]->getPosition().x(),hurt[0]->getPosition().y(),5,Colors::Yellow); }
if (hurt[1]!=NULL) { Broodwar->drawCircle(CoordinateType::Map,hurt[1]->getPosition().x(),hurt[1]->getPosition().y(),5,Colors::Yellow); }

} //for groupID

//Broodwar->sendText("gmE");
}

//---------------------------------------------//
void BasicAIModule::checkResources()
{
	if (myBase==NULL) { return; }

		  Broodwar->sendText("checkResources");
		  //return;
		  double closestGeyserDistance = 30000;
          for(std::set<Unit*>::iterator m=Broodwar->getAllUnits().begin();m!=Broodwar->getAllUnits().end();m++)
		  {
		  //Broodwar->sendText("m:%d",(*m)->getType().isRefinery());
		  if ((*m)->getType().isRefinery()
			  && (*m)->getPlayer()==Broodwar->self()
			  && (*m)->isCompleted()
			  ) {
 
			double dist = BWTA::getGroundDistance(myBase->getPosition(),(*m)->getPosition());
            if (closestGeyser==NULL || (dist < closestGeyserDistance && dist != -1) )
			  { closestGeyser=*m; closestGeyserDistance=dist; }
		  }
		  }
			
		  double closestMineralDistance = 30000;
          for(std::set<Unit*>::iterator m=Broodwar->getMinerals().begin();m!=Broodwar->getMinerals().end();m++)
          {
			double dist = BWTA::getGroundDistance(myBase->getPosition(),(*m)->getPosition());
            if (closestMineral==NULL || (dist < closestMineralDistance && dist != -1) )
			{ closestMineral=*m; closestMineralDistance=dist; }
          }
		  //Broodwar->sendText("min:%.1f",closestMineralDistance);

}