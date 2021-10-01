
#include "particle.hpp"
#include "penningtrap.hpp"


// Constructor
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

  double k = 1;
  //double k = 1.38935333e5;

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

// Evolve the system one time step (dt) using Runge-Kutta 4th order
void PenningTrap::evolve_RK4(double dt){}

// Evolve the system one time step (dt) using Forward Euler
void PenningTrap::evolve_forward_Euler(double dt, int i, double total_time){
  double n = total_time/dt; 
  double m = particles_[i].m_;
  // Define the matrices for the velocity and position for x,y,z-directions
  vec a = vec(3);
  mat v, r; 
  v = mat(3,n).fill(0); //empty matrix with n timesteps in 3D 
  r = mat(3,n).fill(0); 
  vec t = vec(n).fill(0); // empty vector for time wiht n timesteps 
  // initial conditions 
  r.col(0) = particles_[i].r_;
  v.col(0) = particles_[i].v_;

  vec F = total_force(i); 

  //looping through the time for r,v and t with FE method
    for (int i = 0; i < n-1; i++) {
        for (int j = 0; j < 3; j++) {
        a(j) = F(j)/m; 
        v(j,i+1) = v(j,i) + a(j)*dt;
        r(j,i+1) = r(j,i) + v(j,i)*dt;
        t(i+1) = t(i) + dt;
        }
      }
    cout << v << endl;
}
