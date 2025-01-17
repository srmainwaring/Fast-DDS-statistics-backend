// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <gtest_aux.hpp>
#include <gtest/gtest.h>

#include <database/database.hpp>
#include <DatabaseUtils.hpp>

using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;

/**
 * @brief Fixture for the database_load_insert_tests
 */
class database_load_insert_tests : public ::testing::Test
{
public:

    using TestId = PopulateDatabase::TestId;

    void SetUp()
    {
        entities = PopulateDatabase::populate_database(db);
    }

    void check_entity(
            std::shared_ptr<Entity> inserted,
            std::shared_ptr<Entity> loaded)
    {
        ASSERT_TRUE(inserted->id == loaded->id && inserted->kind == loaded->kind &&
                inserted->name == loaded->name && inserted->alias == loaded->alias);
    }

    void check_host(
            std::shared_ptr<Host> inserted,
            std::shared_ptr<Host> loaded)
    {
        check_entity(inserted, loaded);

        ASSERT_TRUE(key_compare(inserted->users, loaded->users));
    }

    void check_user(
            std::shared_ptr<User> inserted,
            std::shared_ptr<User> loaded)
    {
        check_entity(inserted, loaded);

        ASSERT_TRUE(inserted->host->id == loaded->host->id);
        ASSERT_TRUE(key_compare(inserted->processes, loaded->processes));
    }

    void check_process(
            std::shared_ptr<Process> inserted,
            std::shared_ptr<Process> loaded)
    {
        check_entity(inserted, loaded);

        ASSERT_TRUE(inserted->pid == loaded->pid);
        ASSERT_TRUE(inserted->user->id == loaded->user->id);
        ASSERT_TRUE(key_compare(inserted->participants, loaded->participants));
    }

    void check_locator(
            std::shared_ptr<Locator> inserted,
            std::shared_ptr<Locator> loaded)
    {
        check_entity(inserted, loaded);

        ASSERT_TRUE(key_compare(inserted->data_readers, loaded->data_readers));
        ASSERT_TRUE(key_compare(inserted->data_writers, loaded->data_writers));
    }

    void check_domain(
            std::shared_ptr<Domain> inserted,
            std::shared_ptr<Domain> loaded)
    {
        check_entity(inserted, loaded);

        ASSERT_TRUE(key_compare(inserted->topics, loaded->topics));
        ASSERT_TRUE(key_compare(inserted->participants, loaded->participants));
    }

    void check_topic(
            std::shared_ptr<Topic> inserted,
            std::shared_ptr<Topic> loaded)
    {
        check_entity(inserted, loaded);

        ASSERT_TRUE(inserted->data_type == loaded->data_type);
        ASSERT_TRUE(inserted->domain->id == loaded->domain->id);

        ASSERT_TRUE(key_compare(inserted->data_readers, loaded->data_readers));
        ASSERT_TRUE(key_compare(inserted->data_writers, loaded->data_writers));
    }

    void check_participant(
            std::shared_ptr<DomainParticipant> inserted,
            std::shared_ptr<DomainParticipant> loaded)
    {
        check_entity(inserted, loaded);

        ASSERT_TRUE(inserted->qos == loaded->qos && inserted->guid == loaded->guid);

        ASSERT_TRUE(inserted->process->id == loaded->process->id);
        ASSERT_TRUE(inserted->domain->id == loaded->domain->id);

        ASSERT_TRUE(key_compare(inserted->data_readers, loaded->data_readers));
        ASSERT_TRUE(key_compare(inserted->data_writers, loaded->data_writers));
    }

    template<typename T>
    void check_endpoint(
            std::shared_ptr<T> inserted,
            std::shared_ptr<T> loaded)
    {
        check_entity(inserted, loaded);

        ASSERT_TRUE(inserted->qos == loaded->qos && inserted->guid == loaded->guid);

        ASSERT_TRUE(inserted->participant->id == loaded->participant->id);
        ASSERT_TRUE(inserted->topic->id == loaded->topic->id);

        ASSERT_TRUE(key_compare(inserted->locators, loaded->locators));
    }

    template <typename Map>
    bool key_compare (
            Map const& lhs,
            Map const& rhs)
    {
        return lhs.size() == rhs.size()
               && std::equal(lhs.begin(), lhs.end(), rhs.begin(),
                       [](auto a, auto b)
                       {
                           return a.first == b.first;
                       });
    }

    template <typename Map>
    bool map_compare (
            Map const& lhs,
            Map const& rhs)
    {
        // No predicate needed because there is operator== for pairs already.
        return lhs.size() == rhs.size()
               && std::equal(lhs.begin(), lhs.end(),
                       rhs.begin());
    }

    DataBaseTest db;
    std::map<TestId, std::shared_ptr<const Entity>> entities;
};



/**
 * Test to check that load_database() fills the exactly same database that using database.insert()
 * with equivalent entities does. The fixture is necessary for populating a database
 * and then check the results loading the dump of the initially populated database.
 */
TEST_F(database_load_insert_tests, load_insert)
{
    // Dump of the database with inserted datas
    DatabaseDump dump = db.dump_database();

    // Create db_loaded
    DataBaseTest db_loaded;
    db_loaded.load_database(dump);

    // ------------------ Compare map keys ------------------------

    // Compare ID of all maps of both databases.
    // If at() throw an exception, the db_loaded does not contain one Entity which should contain

    // hosts
    ASSERT_TRUE(key_compare(db.hosts(), db_loaded.hosts()));

    // users
    ASSERT_TRUE(key_compare(db.users(), db_loaded.users()));

    // processes
    ASSERT_TRUE(key_compare(db.processes(), db_loaded.processes()));

    // domains
    ASSERT_TRUE(key_compare(db.domains(), db_loaded.domains()));

    // topics
    ASSERT_TRUE(key_compare(db.topics(), db_loaded.topics()));
    for (auto topic : db.topics())
    {
        ASSERT_TRUE(key_compare(topic.second, db_loaded.topics().at(topic.first)));
    }

    // participants
    ASSERT_TRUE(key_compare(db.participants(), db_loaded.participants()));
    for (auto participant : db.participants())
    {
        ASSERT_TRUE(key_compare(participant.second, db_loaded.participants().at(participant.first)));
    }

    // locators
    ASSERT_TRUE(key_compare(db.locators(), db_loaded.locators()));

    // DataWriter
    ASSERT_TRUE(key_compare(db.get_dds_endpoints<DataWriter>(), db_loaded.get_dds_endpoints<DataWriter>()));
    for (auto datawriter : db.get_dds_endpoints<DataWriter>())
    {
        ASSERT_TRUE(key_compare(datawriter.second, db_loaded.get_dds_endpoints<DataWriter>().at(datawriter.first)));
    }

    // DataReader
    ASSERT_TRUE(key_compare(db.get_dds_endpoints<DataReader>(), db_loaded.get_dds_endpoints<DataReader>()));
    for (auto datareader : db.get_dds_endpoints<DataReader>())
    {
        ASSERT_TRUE(key_compare(datareader.second, db_loaded.get_dds_endpoints<DataReader>().at(datareader.first)));
    }

    // locators_by_participant
    ASSERT_TRUE(key_compare(db.locators_by_participant(), db_loaded.locators_by_participant()));
    for (auto locator : db.locators_by_participant())
    {
        ASSERT_TRUE(key_compare(locator.second, db_loaded.locators_by_participant().at(locator.first)));
    }

    // participants_by_locator
    ASSERT_TRUE(key_compare(db.participants_by_locator(), db_loaded.participants_by_locator()));
    for (auto participant : db.participants_by_locator())
    {
        ASSERT_TRUE(key_compare(participant.second, db_loaded.participants_by_locator().at(participant.first)));
    }

    // domains_by_process
    ASSERT_TRUE(key_compare(db.domains_by_process(), db_loaded.domains_by_process()));
    for (auto domain : db.domains_by_process())
    {
        ASSERT_TRUE(key_compare(domain.second, db_loaded.domains_by_process().at(domain.first)));
    }

    // processes_by_domain
    ASSERT_TRUE(key_compare(db.processes_by_domain(), db_loaded.processes_by_domain()));
    for (auto process : db.processes_by_domain())
    {
        ASSERT_TRUE(key_compare(process.second, db_loaded.processes_by_domain().at(process.first)));
    }

    // ------------------ Compare data ------------------------

    // Participants
    for (auto domainIt = db.participants().cbegin(); domainIt != db.participants().cend(); ++domainIt)
    {
        for (auto it = db.participants().at(domainIt->first).cbegin();
                it != db.participants().at(domainIt->first).cend(); ++it)
        {
            DomainParticipantData insertedData =  db.participants().at(domainIt->first).at(it->first)->data;
            DomainParticipantData loadedData =  db_loaded.participants().at(domainIt->first).at(it->first)->data;

            ASSERT_TRUE(map_compare(insertedData.rtps_packets_sent, loadedData.rtps_packets_sent));
            ASSERT_TRUE(map_compare(insertedData.last_reported_rtps_packets_sent_count,
                    loadedData.last_reported_rtps_packets_sent_count));
            ASSERT_TRUE(map_compare(insertedData.rtps_bytes_sent, loadedData.rtps_bytes_sent));
            ASSERT_TRUE(map_compare(insertedData.last_reported_rtps_bytes_sent_count,
                    loadedData.last_reported_rtps_bytes_sent_count));
            ASSERT_TRUE(map_compare(insertedData.rtps_packets_lost, loadedData.rtps_packets_lost));
            ASSERT_TRUE(map_compare(insertedData.last_reported_rtps_packets_lost_count,
                    loadedData.last_reported_rtps_packets_lost_count));
            ASSERT_TRUE(map_compare(insertedData.rtps_bytes_lost, loadedData.rtps_bytes_lost));
            ASSERT_TRUE(map_compare(insertedData.last_reported_rtps_bytes_lost_count,
                    loadedData.last_reported_rtps_bytes_lost_count));
            ASSERT_TRUE(map_compare(insertedData.discovered_entity, loadedData.discovered_entity));
            ASSERT_TRUE(insertedData.pdp_packets == loadedData.pdp_packets);
            ASSERT_TRUE(insertedData.last_reported_pdp_packets == loadedData.last_reported_pdp_packets);
            ASSERT_TRUE(insertedData.edp_packets == loadedData.edp_packets);
            ASSERT_TRUE(insertedData.last_reported_edp_packets == loadedData.last_reported_edp_packets);
            ASSERT_TRUE(map_compare(insertedData.network_latency_per_locator, loadedData.network_latency_per_locator));
        }
    }

    // DataWriter
    for (auto domainIt = db.get_dds_endpoints<DataWriter>().cbegin();
            domainIt != db.get_dds_endpoints<DataWriter>().cend(); ++domainIt)
    {
        for (auto it = db.get_dds_endpoints<DataWriter>().at(domainIt->first).cbegin();
                it != db.get_dds_endpoints<DataWriter>().at(domainIt->first).cend(); ++it)
        {
            DataWriterData insertedData =
                    db.get_dds_endpoints<DataWriter>().at(domainIt->first).at(it->first)->data;
            DataWriterData loadedData =
                    db_loaded.get_dds_endpoints<DataWriter>().at(domainIt->first).at(it->first)->data;

            ASSERT_TRUE(insertedData.publication_throughput == loadedData.publication_throughput);
            ASSERT_TRUE(insertedData.resent_datas == loadedData.resent_datas);
            ASSERT_TRUE(insertedData.last_reported_resent_datas == loadedData.last_reported_resent_datas);
            ASSERT_TRUE(insertedData.heartbeat_count == loadedData.heartbeat_count);
            ASSERT_TRUE(insertedData.last_reported_heartbeat_count == loadedData.last_reported_heartbeat_count);
            ASSERT_TRUE(insertedData.gap_count == loadedData.gap_count);
            ASSERT_TRUE(insertedData.last_reported_gap_count == loadedData.last_reported_gap_count);
            ASSERT_TRUE(insertedData.data_count == loadedData.data_count);
            ASSERT_TRUE(insertedData.last_reported_data_count == loadedData.last_reported_data_count);
            ASSERT_TRUE(map_compare(insertedData.sample_datas, loadedData.sample_datas));
            ASSERT_TRUE(map_compare(insertedData.history2history_latency, loadedData.history2history_latency));
        }
    }

    // DataReader
    for (auto domainIt = db.get_dds_endpoints<DataReader>().cbegin();
            domainIt != db.get_dds_endpoints<DataReader>().cend(); ++domainIt)
    {
        for (auto it = db.get_dds_endpoints<DataReader>().at(domainIt->first).cbegin();
                it != db.get_dds_endpoints<DataReader>().at(domainIt->first).cend(); ++it)
        {
            DataReaderData insertedData =
                    db.get_dds_endpoints<DataReader>().at(domainIt->first).at(it->first)->data;
            DataReaderData loadedData =
                    db_loaded.get_dds_endpoints<DataReader>().at(domainIt->first).at(it->first)->data;

            ASSERT_TRUE(insertedData.subscription_throughput == loadedData.subscription_throughput);
            ASSERT_TRUE(insertedData.acknack_count == loadedData.acknack_count);
            ASSERT_TRUE(insertedData.last_reported_acknack_count == loadedData.last_reported_acknack_count);
            ASSERT_TRUE(insertedData.nackfrag_count == loadedData.nackfrag_count);
            ASSERT_TRUE(insertedData.last_reported_nackfrag_count == loadedData.last_reported_nackfrag_count);

        }
    }

    // ------------------ Compare entities ------------------------

    // Host
    for (auto insertedIt = db.hosts().cbegin(), loadedIt = db_loaded.hosts().cbegin();
            insertedIt != db.hosts().cend() && loadedIt != db_loaded.hosts().cend(); insertedIt++, loadedIt++)
    {
        std::shared_ptr<Host> insertedEntity = insertedIt->second;
        std::shared_ptr<Host> loadedEntity = loadedIt->second;

        check_host(insertedEntity, loadedEntity);
    }

    // Users
    for (auto insertedIt = db.users().cbegin(), loadedIt = db_loaded.users().cbegin();
            insertedIt != db.users().cend() && loadedIt != db_loaded.users().cend(); insertedIt++, loadedIt++)
    {
        std::shared_ptr<User> insertedEntity = insertedIt->second;
        std::shared_ptr<User> loadedEntity = loadedIt->second;

        check_user(insertedEntity, loadedEntity);
    }

    // Processes
    for (auto insertedIt = db.processes().cbegin(), loadedIt = db_loaded.processes().cbegin();
            insertedIt != db.processes().cend() && loadedIt != db_loaded.processes().cend(); insertedIt++, loadedIt++)
    {
        std::shared_ptr<Process> insertedEntity = insertedIt->second;
        std::shared_ptr<Process> loadedEntity = loadedIt->second;

        check_process(insertedEntity, loadedEntity);
    }

    // Locators
    for (auto insertedIt = db.locators().cbegin(), loadedIt = db_loaded.locators().cbegin();
            insertedIt != db.locators().cend() && loadedIt != db_loaded.locators().cend(); insertedIt++, loadedIt++)
    {
        std::shared_ptr<Locator> insertedEntity = insertedIt->second;
        std::shared_ptr<Locator> loadedEntity = loadedIt->second;

        check_locator(insertedEntity, loadedEntity);
    }

    // Domains
    for (auto insertedIt = db.domains().cbegin(), loadedIt = db_loaded.domains().cbegin();
            insertedIt != db.domains().cend() && loadedIt != db_loaded.domains().cend(); insertedIt++, loadedIt++)
    {
        std::shared_ptr<Domain> insertedEntity = insertedIt->second;
        std::shared_ptr<Domain> loadedEntity = loadedIt->second;

        check_domain(insertedEntity, loadedEntity);
    }

    // Participants
    for (auto domainIt = db.participants().cbegin(); domainIt != db.participants().cend(); ++domainIt)
    {
        for (auto insertedIt = db.participants().at(domainIt->first).cbegin(),
                loadedIt = db_loaded.participants().at(domainIt->first).cbegin();
                insertedIt != db.participants().at(domainIt->first).cend() &&
                loadedIt != db_loaded.participants().at(domainIt->first).cend();
                insertedIt++, loadedIt++)
        {
            std::shared_ptr<DomainParticipant> insertedEntity = insertedIt->second;
            std::shared_ptr<DomainParticipant> loadedEntity = loadedIt->second;

            check_participant(insertedEntity, loadedEntity);
        }
    }

    // Datawriters
    for (auto domainIt = db.get_dds_endpoints<DataWriter>().cbegin(); domainIt !=
            db.get_dds_endpoints<DataWriter>().cend();
            ++domainIt)
    {
        for (auto insertedIt = db.get_dds_endpoints<DataWriter>().at(domainIt->first).cbegin(),
                loadedIt = db_loaded.get_dds_endpoints<DataWriter>().at(domainIt->first).cbegin();
                insertedIt != db.get_dds_endpoints<DataWriter>().at(domainIt->first).cend() &&
                loadedIt != db_loaded.get_dds_endpoints<DataWriter>().at(domainIt->first).cend();
                insertedIt++, loadedIt++)
        {
            std::shared_ptr<DataWriter> insertedEntity = insertedIt->second;
            std::shared_ptr<DataWriter> loadedEntity = loadedIt->second;

            check_endpoint(insertedEntity, loadedEntity);
        }
    }

    // Datareaders
    for (auto domainIt = db.get_dds_endpoints<DataReader>().cbegin(); domainIt !=
            db.get_dds_endpoints<DataReader>().cend();
            ++domainIt)
    {
        for (auto insertedIt = db.get_dds_endpoints<DataReader>().at(domainIt->first).cbegin(),
                loadedIt = db_loaded.get_dds_endpoints<DataReader>().at(domainIt->first).cbegin();
                insertedIt != db.get_dds_endpoints<DataReader>().at(domainIt->first).cend() &&
                loadedIt != db_loaded.get_dds_endpoints<DataReader>().at(domainIt->first).cend();
                insertedIt++, loadedIt++)
        {
            std::shared_ptr<DataReader> insertedEntity = insertedIt->second;
            std::shared_ptr<DataReader> loadedEntity = loadedIt->second;

            check_endpoint(insertedEntity, loadedEntity);
        }
    }

    // Topics
    for (auto domainIt = db.topics().cbegin(); domainIt != db.topics().cend(); ++domainIt)
    {
        for (auto insertedIt = db.topics().at(domainIt->first).cbegin(),
                loadedIt = db_loaded.topics().at(domainIt->first).cbegin();
                insertedIt != db.topics().at(domainIt->first).cend() &&
                loadedIt != db_loaded.topics().at(domainIt->first).cend();
                insertedIt++, loadedIt++)
        {
            std::shared_ptr<Topic> insertedEntity = insertedIt->second;
            std::shared_ptr<Topic> loadedEntity = loadedIt->second;

            check_topic(insertedEntity, loadedEntity);
        }
    }

    // domains_by_process
    auto domain_by_process = db.domains_by_process();
    auto loaded_domain_by_process = db_loaded.domains_by_process();
    for (auto domainIt = domain_by_process.cbegin(); domainIt != domain_by_process.cend(); ++domainIt)
    {
        for (auto insertedIt = domain_by_process.at(domainIt->first).cbegin(),
                loadedIt = loaded_domain_by_process.at(domainIt->first).cbegin();
                insertedIt != domain_by_process.at(domainIt->first).cend() &&
                loadedIt != loaded_domain_by_process.at(domainIt->first).cend();
                insertedIt++, loadedIt++)
        {
            std::shared_ptr<Domain> insertedEntity = insertedIt->second;
            std::shared_ptr<Domain> loadedEntity = loadedIt->second;

            check_domain(insertedEntity, loadedEntity);
        }
    }

    // processes_by_domain_
    auto processes_by_domain = db.processes_by_domain();
    auto loaded_processes_by_domain = db_loaded.processes_by_domain();
    for (auto domainIt = processes_by_domain.cbegin(); domainIt != processes_by_domain.cend(); ++domainIt)
    {
        for (auto insertedIt = processes_by_domain.at(domainIt->first).cbegin(),
                loadedIt = loaded_processes_by_domain.at(domainIt->first).cbegin();
                insertedIt != processes_by_domain.at(domainIt->first).cend() &&
                loadedIt != loaded_processes_by_domain.at(domainIt->first).cend();
                insertedIt++, loadedIt++)
        {
            std::shared_ptr<Process> insertedEntity = insertedIt->second;
            std::shared_ptr<Process> loadedEntity = loadedIt->second;

            check_process(insertedEntity, loadedEntity);
        }
    }

    // participants_by_locator
    auto participants_by_locator = db.participants_by_locator();
    auto loaded_participants_by_locator = db_loaded.participants_by_locator();
    for (auto domainIt = participants_by_locator.cbegin(); domainIt != participants_by_locator.cend();
            ++domainIt)
    {
        for (auto insertedIt = participants_by_locator.at(domainIt->first).cbegin(),
                loadedIt = loaded_participants_by_locator.at(domainIt->first).cbegin();
                insertedIt != participants_by_locator.at(domainIt->first).cend() &&
                loadedIt != loaded_participants_by_locator.at(domainIt->first).cend();
                insertedIt++, loadedIt++)
        {
            std::shared_ptr<DomainParticipant> insertedEntity = insertedIt->second;
            std::shared_ptr<DomainParticipant> loadedEntity = loadedIt->second;

            check_participant(insertedEntity, loadedEntity);
        }
    }

    // locators_by_participant
    auto locators_by_participant = db.locators_by_participant();
    auto loaded_locators_by_participant = db_loaded.locators_by_participant();
    for (auto domainIt = locators_by_participant.cbegin(); domainIt != locators_by_participant.cend();
            ++domainIt)
    {
        for (auto insertedIt = locators_by_participant.at(domainIt->first).cbegin(),
                loadedIt = loaded_locators_by_participant.at(domainIt->first).cbegin();
                insertedIt != locators_by_participant.at(domainIt->first).cend() &&
                loadedIt != loaded_locators_by_participant.at(domainIt->first).cend();
                insertedIt++, loadedIt++)
        {
            std::shared_ptr<Locator> insertedEntity = insertedIt->second;
            std::shared_ptr<Locator> loadedEntity = loadedIt->second;

            check_locator(insertedEntity, loadedEntity);
        }
    }

    // Compare next_id_ of both databases
    ASSERT_EQ(db.next_id(), db_loaded.next_id());

    // Compare dump of both databases
    ASSERT_EQ(dump, db_loaded.dump_database());
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
