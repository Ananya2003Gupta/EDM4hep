//////////////////////////////////////////////////////////////////////////
// Adapted from a HepMC by  Matt.Dobbs@Cern.CH, Feb 2000
// Example of building an event and a particle data table from scratch
// and converting it to the edm4hep data model
//////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <memory>
#include "HepMC/GenEvent.h"
#include "HepPDT/ParticleID.hh"
#include "edm4hep/MCParticleCollection.h"
#include "podio/EventStore.h"
#include "podio/ROOTWriter.h"

using namespace HepMC;

int main() {
  // Part 1: Using the HepMC example code to "generate" an event
      //
      // In this example we will place the following event into HepMC "by hand"
      //
      //     name status pdg_id  parent Px       Py    Pz       Energy      Mass
      //  1  !p+!    3   2212    0,0    0.000    0.000 7000.000 7000.000    0.938
      //  2  !p+!    3   2212    0,0    0.000    0.000-7000.000 7000.000    0.938
      //=========================================================================
      //  3  !d!     3      1    1,1    0.750   -1.569   32.191   32.238    0.000
      //  4  !u~!    3     -2    2,2   -3.047  -19.000  -54.629   57.920    0.000
      //  5  !W-!    3    -24    1,2    1.517   -20.68  -20.605   85.925   80.799
      //  6  !gamma! 1     22    1,2   -3.813    0.113   -1.833    4.233    0.000
      //  7  !d!     1      1    5,5   -2.445   28.816    6.082   29.552    0.010
      //  8  !u~!    1     -2    5,5    3.962  -49.498  -26.687   56.373    0.006

      // now we build the graph, which will look like
      //                       p7                         #
      // p1                   /                           #
      //   \v1__p3      p5---v4                           #
      //         \_v3_/       \                           #
      //         /    \        p8                         #
      //    v2__p4     \                                  #
      //   /            p6                                #
      // p2                                               #
      //                                                  #

      // First create the event container, with Signal Process 20, event number 1
      //
      std::unique_ptr<GenEvent> evt = std::make_unique<GenEvent>( 20, 1 );
      // define the units
      evt->use_units(HepMC::Units::GEV, HepMC::Units::MM);
      //
      // create vertex 1 and vertex 2, together with their inparticles
      GenVertex* v1 = new GenVertex();
      evt->add_vertex( v1 );
      v1->add_particle_in( new GenParticle( FourVector(0,0,7000,7000),
                 2212, 3 ) );
      GenVertex* v2 = new GenVertex();
      evt->add_vertex( v2 );
      v2->add_particle_in( new GenParticle( FourVector(0,0,-7000,7000),
                 2212, 3 ) );
      //
      // create the outgoing particles of v1 and v2
      GenParticle* p3 = 
    new GenParticle( FourVector(.750,-1.569,32.191,32.238), 1, 3 );
      v1->add_particle_out( p3 );
      GenParticle* p4 = 
    new GenParticle( FourVector(-3.047,-19.,-54.629,57.920), -2, 3 );
      v2->add_particle_out( p4 );
      //
      // create v3
      GenVertex* v3 = new GenVertex();
      evt->add_vertex( v3 );
      v3->add_particle_in( p3 );
      v3->add_particle_in( p4 );
      v3->add_particle_out( 
    new GenParticle( FourVector(-3.813,0.113,-1.833,4.233 ), 22, 1 )
    );
      GenParticle* p5 = 
    new GenParticle( FourVector(1.517,-20.68,-20.605,85.925), -24,3);
      v3->add_particle_out( p5 );
      //
      // create v4
      GenVertex* v4 = new GenVertex(FourVector(0.12,-0.3,0.05,0.004));
      evt->add_vertex( v4 );
      v4->add_particle_in( p5 );
      v4->add_particle_out( 
    new GenParticle( FourVector(-2.445,28.816,6.082,29.552), 1,1 )
    );
      v4->add_particle_out( 
    new GenParticle( FourVector(3.962,-49.498,-26.687,56.373), -2,1 )
    );
      //    
      // tell the event which vertex is the signal process vertex
      evt->set_signal_process_vertex( v3 );
      // the event is complete, we now print it out to the screen
      evt->print();

      // now clean-up by deleteing all objects from memory
      //
      // deleting the event deletes all contained vertices, and all particles
      // contained in those vertices



  //Part2: Convert the particles and write to file
    auto store = podio::EventStore();
    auto writer = podio::ROOTWriter("edm4hep_testhepmc.root", &store);
    auto& particles       = store.create<edm4hep::MCParticleCollection>("testparticles");
    writer.registerForWrite("testparticles");
    for (auto particle_i = evt->particles_begin(); particle_i != evt->particles_end(); ++particle_i) {
      std::cout << "Converting particle with Pdg_ID" (*particle_i)->pdg_id() << std::endl;
      auto part = edm4hep::MCParticle();
      part.setPDG((*particle_i)->pdg_id());
      part.setGeneratorStatus((*particle_i)->status());
      // look up charge from pdg_id
      HepPDT::ParticleID particleID((*particle_i)->pdg_id());
      part.setCharge(particleID.charge());
      //  convert momentum
      auto tmp = (*particle_i)->momentum();
      part.setMomentum(edm4hep::FloatThree(tmp.px(), tmp.py(), tmp.pz()));
      particles.push_back(part);

      //TODO: mother/daughter links

      writer.writeEvent();
      store.clearCollections();

    }
    writer.finish();

  return 0;
}