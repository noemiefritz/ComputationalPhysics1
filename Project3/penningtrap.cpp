
#include "particle.hpp"
#include "penningtrap.hpp"


// Constructor
//double B0=96.5, double V0=9.65e8, double d=1e4
PenningTrap::PenningTrap(vector<Particle> particles, double B0, double V0, double d){

  particles_ = particles;
  B0_ = B0;
  V0_ = V0;
  d_ = d;
}

// Add a particle to the trap
void PenningTrap::add_particle(Particle particle){
  particles_.push_back(particle);
}

// Counts number of particles in particles
int PenningTrap::particle_count()
    {
      return particles_.size();
    }

// Prints number of particles in particles
void PenningTrap::info()
    {
      std::cout << "number of particles = " << particle_count() << endl;

    }

// External electric field at point r=(x,y,z)
vec PenningTrap::external_E_field(int i){
  vec r = particles_[i].r_;
  vec E = vec(3);
  E(0) = (V0_/(d_*d_))*(r(0));
  E(1) = (V0_/(d_*d_))*(r(1));
  E(2) = (V0_/(d_*d_))*(-2*r(2));
  return E;
}

// External magnetic field at point r=(x,y,z)
vec PenningTrap::external_B_field(int i){
  vec v = particles_[i].v_;
  int q = particles_[i].q_;
  vec B = vec(3);
  B(0) = B0_*q*v(1);
  B(1) = -B0_*q*v(0) ;
  B(2) = 0;
  return B;
}

// Force on particle_i from particle_j
vec PenningTrap::force_particle(int i, int j){
  vec r_i = particles_[i].r_;
  vec r_j = particles_[j].r_;

  int q_i = particles_[i].q_;
  int q_j = particles_[j].q_;

  double m_i = particles_[i].m_;

  double k = 1.38935333e5;

  vec C = vec(3);
  C(0) = k*(q_i/m_i)*q_j*(r_i(0)-r_j(0))/pow(sqrt(pow((r_i(0)-r_j(0)),2) + pow((r_i(1)-r_j(1)),2) + pow((r_i(2)-r_j(2)),2)),3);
  C(1) = k*(q_i/m_i)*q_j*(r_i(1)-r_j(1))/pow(sqrt(pow((r_i(0)-r_j(0)),2) + pow((r_i(1)-r_j(1)),2) + pow((r_i(2)-r_j(2)),2)),3);
  C(2) = k*(q_i/m_i)*q_j*(r_i(2)-r_j(2))/pow(sqrt(pow((r_i(0)-r_j(0)),2) + pow((r_i(1)-r_j(1)),2) + pow((r_i(2)-r_j(2)),2)),3);
  return C;
}

// The total force on particle_i from the external fields
vec PenningTrap::total_force_external(int i){
  vec F_ext = vec(3);
  F_ext = external_E_field(i) + external_B_field(i);
  return F_ext;
}

// The total force on particle_i from the other particles
vec PenningTrap::total_force_particles(int i){
  int num = PenningTrap::particle_count();
  vec F_tot_particle =  {0,0,0};

  for (int j=0; j<num; j++){
    if (j!= i){
      F_tot_particle =  F_tot_particle + force_particle(i,j);
    }
  }
  return F_tot_particle;
}

// The total force on particle_i from both external fields and other particles
vec PenningTrap::total_force(int i){
  vec F_tot = vec(3);
  F_tot = total_force_external(i) + total_force_particles(i);
  return F_tot;
}

void PenningTrap::simulation(double dt, double total_time){
  int n = (int) (total_time/dt);
  int n_par = particles_.size();

  // Define the matrices for the velocity and position for x,y,z-directions
  t = vec(n).fill(0); // empty vector for time wiht n timesteps
  v = cube(3,n_par,n).fill(0); //empty matrix with n timesteps in 3D
  r = cube(3,n_par,n).fill(0);
  // Evolve the system one time step (dt) using Forward Euler and RK4
  for (int j=0; j<n-1; j++){
    for (int i=0; i< particles_.size(); i++){
      evolve_RK4(dt, i, j);
      //evolve_forward_Euler(dt, i, j);
    }
  }
  vec time = linspace(0, total_time, n);
  time.save("time.bin");
  r.save("position.bin");
  v.save("velocity.bin");
  //t_tot.save("t_tot.bin");
}
// Evolve the system one time step (dt) using Runge-Kutta 4th order
void PenningTrap::evolve_RK4(double dt, int i, int j){
  double m = particles_[i].m_;
  vec K1v, K2v, K3v, K4v, K1r, K2r, K3r, K4r, v_old, r_old, a;

  a = vec(3);
  //r.slice(j).col(i)
  // initial conditions
  r.slice(j).col(i) = particles_[i].r_;
  v.slice(j).col(i) = particles_[i].v_;

  vec F = total_force(i);
  r_old = r.slice(j).col(i);
  v_old = v.slice(j).col(i);


  a = F/m;
  //No uptades for v and r
  K1v = dt*a;
  K1r = dt*particles_[i].v_;
  //1. update for v and r
  particles_[i].r_ = r_old + K1r/2;
  particles_[i].v_ = v_old + K1v/2;

  a = total_force(i)/m;
  K2v = dt*a; //use a(i) when implementing total force
  K2r = dt*particles_[i].v_;


  //2.update for v and r
  particles_[i].r_ = r_old + K2r/2;
  particles_[i].v_ = v_old + K2v/2;

  a = total_force(i)/m;
  K3v = dt*a;
  K3r = dt*particles_[i].v_;
  //3.update for v and r
  particles_[i].r_ = r_old + K3r;
  particles_[i].v_ = v_old + K3v;

  a = total_force(i)/m;
  K4v = dt*a;
  K4r = dt*particles_[i].v_;


  //4. update for v and r
  v.slice(j+1).col(i) = v_old + (1/6.0)*(K1v + 2*K2v + 2*K3v + K4v);
  r.slice(j+1).col(i) = r_old + (1/6.0)*(K1r + 2*K2r + 2*K3r + K4r);

  particles_[i].r_ = r.slice(j+1).col(i);
  particles_[i].v_ = v.slice(j+1).col(i);


  //ofstream file1;
  //file1.open("single_particle_movement_RK4.txt", ios::out); //opens file1 in out/write mode
  //file1 << setw(25) << "x" << setw(25) << "y" << setw(25) << "z" << setw(25) << "v_x" << setw(25) << "v_y" << setw(25) << "v_z"<< endl;

  //for (int j = 0; j < n-1; j++){
  //  file1 << setw(25) << r(j, particles_[i].size, 0);
  //  file1 << setw(25) << r(j, particles_[i].size, 1);
  //  file1 << setw(25) << r(j, particles_[i].size, 2);
  //  file1 << setw(25) << v(j, particles_[i].size, 0);
  //  file1 << setw(25) << v(j, particles_[i].size, 1);
  //  file1 << setw(25) << v(j, particles_[i].size, 2);
  //  file1 << endl;
  //}

  //file1.close();
  return;

}

void PenningTrap::evolve_forward_Euler(double dt, int i, int j){
  double m = particles_[i].m_;
  vec a = vec(3);
  // initial conditions
  r.slice(j).col(i) = particles_[i].r_;
  v.slice(j).col(i) = particles_[i].v_;

  vec F = total_force(i);

  a = F/m;
  //cout << a << endl;
  //cout << v.slice(j).col(i) << endl;
  v.slice(j+1).col(i) = particles_[i].v_ + a*dt;//v.slice(j).col(i) + a*dt;
  r.slice(j+1).col(i) = r.slice(j).col(i) + v.slice(j).col(i)*dt;
  t(j+1) = t(j) + dt;


}
