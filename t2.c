///////
////// Rus Mihai Tudorel
////// group 30221 CTI
/////  23.05.2020 22:30
///// the reaction : 2HCl + CaO ---> CaCl2 + H2O
// Im sorry I haven t commentig all the code and haven t write the solution.txt ,
// but tomorrow I will submit another source with all the comments and the txt

#include <pthread.h>
#include <stdlib.h>
#include<stdio.h>
#include<string.h>
#include<semaphore.h>
#include<unistd.h>
#include<ctype.h>

int N;

int atoms_of_Ca;
int atoms_of_O;
int atoms_of_H;
int atoms_of_Cl;

int molecule_CaO;
int molecule_HCl;

int molecule_water;//contor for water molecules
int molecule_CaCl2;//contor for calcium chlorhide molecules

pthread_mutex_t mutexCaO;//mutex for calcium oxide
pthread_barrier_t barrierCaO;//barrier for calcium oxide

pthread_mutex_t mutexHCl;//mutex for hydrochloric acid
pthread_barrier_t barrierHCl;//barrier for hydrochloric acid

pthread_t water_thread;//thread for watter
pthread_mutex_t mutexWater;//mutex for water
pthread_barrier_t barrierWater;// barrier for water

pthread_t CaCl2_thread;
pthread_mutex_t mutexCaCl2;
pthread_barrier_t barrierCaCl2;

pthread_t HCl_thread;//thread for hydrochloric acid
pthread_t CaO_thread;//thread for calcium oxide

//Firstly I build the atoms of H2,Ca,O2 and Cl with threads where I used mutexes,semaphores and locks
//to create the molecules of HCl and CaO;
//Once I created 2 molecules of HCl and one of CaO I am using another threads and threat the molecules as a new atoms
//and with that 2 threads (HCl_thread and CaO_thread) I create another thread(water_thread) with mutex and barrier
//where of course I use semaphores like CaO_sem and HCl_sem to keep the number of molecules updated.
//I keep the number of molecules of water in the variable molecule_water and then I compare it with N.
//Same pattern for the molecules of Calcium chloride, but there doesn t matter how manny molecules are created.


sem_t Calcium_sem;//semaphore for atom of Calcium
sem_t Oxygen_sem;//semaphore for atom of Oxygen

sem_t Hydrogen_sem;//semaphore for atom of Hydrogen
sem_t Chlorine_sem;//semaphore for atom of Chlorine

sem_t CaO_sem;//semaphore for molecule of CaO
sem_t HCl_sem;//semaphore for molecule of HCl

int idOxygen;
int idCalcium;
int idHydrogen;
int idChlorine;
//these id's are auxiliary and I used them to keep increasing the number of atoms and molecules printed

int idCaO;
int idHCl;


//water stuff
/// first reactant of the reaction

void check(){
    if(molecule_water==N) {//here is the finish condition
        printf("You Sir just created %d molecules of water. \n",molecule_water);
        exit(0);
    }
}
void* water(void* message){
    molecule_water++;//here I am increasing the number of water molecules
    printf("%s Molecule id: %d\n\n",(char*)message,molecule_water);//here I print the number of molecules created so far
    check();//here I check if the N is reached every time the thread is called

}
void molecule_of_water(){
    if(pthread_create(&water_thread, NULL, water,(void *) "A molecule of water has been created.\n")!=0) {//here I call the thread with the message
        printf("There is no molecule of water.");//here I print if I cant create a new thread
       exit(1);
    }

}


//calcium chlorhide stuff
///second reactant of the reaction

void *CaCl2(void* message){
    molecule_CaCl2++;//here I am increasing the number of CaCl2 molecules
    printf("%s",(char*)message);
}
void molecule_of_CaCl2(){
    if(pthread_create(&CaCl2_thread,NULL,CaCl2,(void*)"The reaction :\n 2HCl + Cao ---> CaCl2 + H2O \n\nA molecule of Calcium chlorhide has been created.\n")!=0){
        //here I call the thread
        printf("There is no molecule of Calcium chlorhide.");
        exit(1);
    }
}

//calcium oxide stuff
/// first molecule we need to start the reaction

void* CaO(void* message) {
    printf("%s with number : %d\n",(char*)message, ++idCaO);//here I raise the number of printed molecules
    pthread_mutex_lock(&mutexWater);//lock mutex because I need to create a water molecule
    pthread_mutex_lock(&mutexCaCl2);//lock the mutex in order to create a CaCl2// start of the critic zone
    molecule_CaO++;//here I increase the number of molecules used in threads
    if (molecule_HCl>= 2) {//we need 2 molecules of HCl to start the reaction
        sem_post(&HCl_sem);
        sem_post(&HCl_sem);//erase 2 atoms of HCl from the queue
        molecule_HCl-=2;//decrement the number of HCl molecules
        sem_post(&CaO_sem);//erase only one molecule of CaO from the queue
        molecule_CaO--;
    } else {
        pthread_mutex_unlock(&mutexWater);// end of critic zone
        pthread_mutex_unlock(&mutexCaCl2);
    }
    sem_wait(&CaO_sem);// waiting for CaO to form the molecule

    molecule_of_CaCl2();//call the thread function
    molecule_of_water();//call the thread function

    pthread_barrier_wait(&barrierCaCl2);//waiting for the barrier of CaCl2
    pthread_mutex_unlock(&mutexCaCl2);//end of critic zone if we dont go on that else

    pthread_barrier_wait(&barrierWater);//waiting for the barrier of water
    pthread_mutex_unlock(&mutexWater);
}
void molecule_of_CaO(char* message){//the function who calls the thread
    if(pthread_create(&CaO_thread, NULL, CaO,(void *) "A molecule of CaO has been created.")!=0) {//the call
        printf("There is no molecule of CaO\n");
        exit(1);
    }

}

/// creation of Calcium atom
void* Calcium(void* message) {//the function for Calcium
    printf("%s with number : %d\n",(char*)message, ++idCalcium);//id for Calcium atoms printed
    pthread_mutex_lock(&mutexCaO);//start of critic zone
    atoms_of_Ca++;//number of atoms increased in threads
    if (atoms_of_O >= 1) {//I need only one molecule of Oxygen
        sem_post(&Oxygen_sem);//erase one atom of Oxygen
        atoms_of_O--;//decrement the value
        sem_post(&Calcium_sem);//erase one atom of Calcium
        atoms_of_Ca--;//decrement the value
    } else {
        pthread_mutex_unlock(&mutexCaO);//end of critic zone
    }

    sem_wait(&Calcium_sem);//waiting for Calcium to form the molecule

    pthread_barrier_wait(&barrierCaO);//waiting for barrier in CaO to form the molecule

}


/// the creation of Oxygen atom
void* Oxygen(void* message){
    printf("%s with number : %d\n",(char*)message,++idOxygen);
    pthread_mutex_lock(&mutexCaO);
    atoms_of_O++;
    if(atoms_of_Ca>=1){
        sem_post(&Calcium_sem);
        atoms_of_Ca--;
        sem_post(&Oxygen_sem);
        atoms_of_O--;
    }
    else{
        pthread_mutex_unlock(&mutexCaO);
    }
    sem_wait(&Oxygen_sem);

    molecule_of_CaO((char*)"A molecule of CaO has been created.\n");

    pthread_barrier_wait(&barrierCaO);

    pthread_mutex_unlock(&mutexCaO);

}
/// for the rest of the code till int main() I use the same method to create the HCl molecule
//Hydrochloric acid stuff
void* HCl(void* message) {
    printf("%s with number : %d\n",(char*)message, ++idHCl);
    pthread_mutex_lock(&mutexCaCl2);
    pthread_mutex_lock(&mutexWater);
    molecule_HCl++;
    if (molecule_CaO>= 1 && molecule_HCl>=1) {
        sem_post(&HCl_sem);
        sem_post(&HCl_sem);
        molecule_HCl-=2;
        sem_post(&CaO_sem);
        molecule_CaO--;
    } else {
        pthread_mutex_unlock(&mutexCaCl2);
        pthread_mutex_unlock(&mutexWater);
    }
    sem_wait(&HCl_sem);

    pthread_barrier_wait(&barrierCaCl2);

    pthread_barrier_wait(&barrierWater);

}
void molecule_of_HCl(char* message){
    if(pthread_create(&HCl_thread, NULL, HCl,(void *) "A molecule of HCl has been created.")!=0) {
        printf("There is no molecule of HCl\n");
        exit(1);
    }

}

void *Hydrogen(void *message){
    printf("%s with number : %d\n",(char*)message,++idHydrogen);
    pthread_mutex_lock(&mutexHCl);
    atoms_of_H++;
    if(atoms_of_Cl>=1){
        sem_post(&Chlorine_sem);
        atoms_of_Cl--;
        sem_post(&Hydrogen_sem);
        atoms_of_H--;
    }
    else{
        pthread_mutex_unlock(&mutexHCl);
    }
    sem_wait(&Hydrogen_sem);
    molecule_of_HCl((char*)"\nA molecule of HCl has been created.\n");

    pthread_barrier_wait(&barrierHCl);
    pthread_mutex_unlock(&mutexHCl);
}

void* Chlorine(void* message){
    printf("%s with number : %d\n",(char*)message,++idChlorine);
    pthread_mutex_lock(&mutexHCl);
    atoms_of_Cl++;
    if(atoms_of_H>=1){
        sem_post(&Hydrogen_sem);
        atoms_of_H--;
        sem_post(&Chlorine_sem);
        atoms_of_Cl--;
    }
    else{
        pthread_mutex_unlock(&mutexHCl);
    }
    sem_wait(&Chlorine_sem);
    pthread_barrier_wait(&barrierHCl);
}


// the START of the reaction
int main(int argc,char **argv)
{
    if(argc!=2) {//here I test the number of arguments
        printf("You sir just typed a wrong information here.\n");
        printf("Please Sir just write ./t2 2\n");
        exit(1);
    }
    int time=0;

   N=atoi(argv[1]);//converting the argument to int


    pthread_t Calcium_thread;
    pthread_t Oxygen_thread;
    pthread_t Hydrogen_thread;
    pthread_t Chlorine_thread;
    //creating the threads for the atoms

    pthread_mutex_init(&mutexCaO,NULL);
    pthread_mutex_init(&mutexHCl,NULL);
    pthread_mutex_init(&mutexCaCl2,NULL);
    pthread_mutex_init(&mutexWater,NULL);

    //init for mutexes used to create the molecules




    sem_init(&Calcium_sem,0,0);
    sem_init(&Oxygen_sem,0,0);
    sem_init(&Hydrogen_sem,0,0);
    sem_init(&Chlorine_sem,0,0);

    //init for semaphores used to wait for each atom

    sem_init(&HCl_sem,0,0);
    sem_init(&CaO_sem,0,0);

    //init for semaphores used to wait for each molecule

    pthread_barrier_init(&barrierCaO,0,2);// init with 2 because I need only one oxygen and only one calcium atom
    pthread_barrier_init(&barrierHCl,0,2);// same but with hydrogen and chlorine
    pthread_barrier_init(&barrierCaCl2,0,3); // init with 3 because I need 2 atoms of chlorine and only one of calcium
    pthread_barrier_init(&barrierWater,0,3);// init with 3 because I need 2 atoms of hydrogen and only one of oxygen

    printf("Start time:%d\n",time);
    for(int i=0;i<4*N;i++){//because I need only 4 types of molecules to form the reaction
        if(i%4==0){
            if(pthread_create(&Hydrogen_thread,NULL,Hydrogen,(void*)"An atom of Hydrogen has been created.")!=0){//call the hydrogen thread
                printf("You Sir do not have enough Hydrogen atoms.\n");
                exit(1);

            }
            if(pthread_create(&Hydrogen_thread,NULL,Hydrogen,(void*)"An atom of Hydrogen has been created.")!=0){//call it twice because I need two atoms
                printf("You Sir do not have enough Hydrogen atoms.\n");
                exit(1);

            }
            //sleep(2);
        }
        if(i%4==1){
            if(pthread_create(&Chlorine_thread,NULL,Chlorine,(void*)"An atom of Chlorine has been created.")!=0){//call the chlorine thread
                printf("You Sir do not have enough Chlorine atoms.\n");
                exit(1);
            }
            if(pthread_create(&Chlorine_thread,NULL,Chlorine,(void*)"An atom of Chlorine has been created.")!=0){//call it twice
                printf("You Sir do not have enough Chlorine atoms.\n");
                exit(1);
            }
            //sleep(1);
        }
        if(i%4==2){
            if(pthread_create(&Calcium_thread, NULL, Calcium,(void *)"An atom of Calcium has been created.")!=0) {//call the calcium thread
                printf("You Sir do not have enough Calcium atoms.\n");
                exit(1);

            }
            //sleep(1);

        }
        if(i%4==3){
            if(pthread_create(&Oxygen_thread,NULL,Oxygen,(void*)"An atom of Oxygen has been created.")!=0){//call the oxygen thread
                printf("You Sir do not have enough Oxygen atoms.\n");
                exit(1);
            }
            sleep(1);
        }



    }

    //here I join every thread
    pthread_join(Calcium_thread,NULL);
    pthread_join(Oxygen_thread,NULL);
    pthread_join(Hydrogen_thread,NULL);
    pthread_join(Chlorine_thread,NULL);
    pthread_join(HCl_thread,NULL);
    pthread_join(CaO_thread,NULL);

    //here I destroy the threads,semaphores,mutexes and barriers
    pthread_mutex_destroy(&mutexCaO);
    pthread_mutex_destroy(&mutexHCl);
    pthread_mutex_destroy(&mutexCaCl2);
    pthread_mutex_destroy(&mutexWater);

    sem_destroy(&Calcium_sem);
    sem_destroy(&Oxygen_sem);
    sem_destroy(&Hydrogen_sem);
    sem_destroy(&Chlorine_sem);
    sem_destroy(&HCl_sem);
    sem_destroy(&CaO_sem);

    pthread_barrier_destroy(&barrierCaO);
    pthread_barrier_destroy(&barrierHCl);
    pthread_barrier_destroy(&barrierCaCl2);
    pthread_barrier_destroy(&barrierWater);


    exit(0);
}