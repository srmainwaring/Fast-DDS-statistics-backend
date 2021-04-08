/* Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file entities.hpp
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_ENTITIES_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_ENTITIES_HPP_

#include "data.hpp"
#include <fastdds-statistics-backend/types/types.hpp>
#include <fastdds-statistics-backend/types/EntityId.hpp>

#include <string>

namespace eprosima {
namespace statistics_backend {
namespace database {

struct User;
struct Process;
struct DomainParticipant;
struct Topic;
struct DataReader;
struct DataWriter;
struct Locator;

/*
 * Base struct for all the database possible entities.
 *
 * Entities constitute the nodes of the database. They hold the relation they have
 * both upwards and downwards, plus they hold the statistical data reported by Fast
 * DDS Statistics Module.
 */
struct Entity
{
    Entity(
            EntityKind entity_kind = EntityKind::INVALID,
            std::string entity_name = "INVALID")
        : kind(entity_kind)
        , name(entity_name)
    {
    }

    //! The unique identification of the entity
    EntityId id;

    //! The kind of entity
    EntityKind kind;

    //! A user defined human-readable name for the entity
    std::string name;
};

/*
 * Host entities hold data about the real host involved in the communication
 */
struct Host : Entity
{
    Host(
            std::string host_name)
        : Entity(EntityKind::HOST, host_name)
    {
    }

    /*
     * Collection of users within the host which are involved in the communication.
     * The collection is ordered by the EntityId of the user nodes.
     */
    std::map<EntityId, std::shared_ptr<User>> users;
};

/*
 * User entities hold data about the host users involved in the communication
 */
struct User : Entity
{
    User(
            std::string user_name,
            std::shared_ptr<Host> user_host)
        : Entity(EntityKind::USER, user_name)
        , host(user_host)
    {
    }

    //! Reference to the Host in which this user runs
    std::shared_ptr<Host> host;

    /*
     * Collection of processes within the host which are involved in the communication.
     * The collection is ordered by the EntityId of the process nodes.
     */
    std::map<EntityId, std::shared_ptr<Process>> processes;
};

/*
 * Process entities hold data about the computing processes involved in the communication.
 */
struct Process : Entity
{
    Process(
            std::string process_name,
            std::string process_id,
            std::shared_ptr<User> process_user)
        : Entity(EntityKind::PROCESS, process_name)
        , pid(process_id)
        , user(process_user)
    {
    }

    //! The PID of the process
    std::string pid;

    //! Reference to the User in which this process runs.
    std::shared_ptr<User> user;

    /*
     * Collection of DomainParticipant within the process which are involved in the communication.
     * The collection is ordered by the EntityId of the DomainParticipant nodes.
     */
    std::map<EntityId, std::shared_ptr<DomainParticipant>> participants;
};

/*
 * Domain entities hold data about the DDS domains or Discovery Server networks involved in the
 * communication.
 */
struct Domain : Entity
{
    Domain(
            std::string domain_name)
        : Entity(EntityKind::DOMAIN, domain_name)
    {
    }

    /*
     * Collection of Topics within the Domain which are either published, subscribed, or both.
     * The collection is ordered by the EntityId of the Topic nodes.
     */
    std::map<EntityId, std::shared_ptr<Topic>> topics;

    /*
     * Collection of DomainParticipant within the Domain which are involved in the communication.
     * The collection is ordered by the EntityId of the DomainParticipant nodes.
     */
    std::map<EntityId, std::shared_ptr<DomainParticipant>> participants;
};

/*
 * Base class for the DDS Entities.
 */
struct DDSEntity : Entity
{
    DDSEntity(
            EntityKind entity_kind = EntityKind::INVALID,
            std::string dds_entity_name = "INVALID",
            Qos dds_entity_qos = {},
            std::string dds_entity_guid = "|GUID UNKNOWN|")
        : Entity(entity_kind, dds_entity_name)
        , qos(dds_entity_qos)
        , guid(dds_entity_guid)
    {
    }

    //! Quality of Service configuration of the entities in a tree structure.
    Qos qos;

    //! The DDS GUID of the entity
    std::string guid;
};

/*
 * DomainParticipant entities hold data about the DomainParticipant involved in the communication.
 */
struct DomainParticipant : DDSEntity
{
    DomainParticipant(
            std::string participant_name,
            Qos participant_qos,
            std::string participant_guid,
            std::shared_ptr<Process> participant_process,
            std::shared_ptr<Domain> participant_domain)
        : DDSEntity(EntityKind::PARTICIPANT, participant_name, participant_qos, participant_guid)
        , process(participant_process)
        , domain(participant_domain)
    {
    }

    //! Reference to the Process in which this DomainParticipant runs.
    std::shared_ptr<Process> process;

    //! Reference to the Domain in which this DomainParticipant runs.
    std::shared_ptr<Domain> domain;

    /*
     * Collection of DataReaders within the DomainParticipant which are involved in the communication.
     * The collection is ordered by the EntityId of the DataReader nodes.
     */
    std::map<EntityId, std::shared_ptr<DataReader>> data_readers;

    /*
     * Collection of DataWriters within the DomainParticipant which are involved in the communication.
     * The collection is ordered by the EntityId of the DataWriter nodes.
     */
    std::map<EntityId, std::shared_ptr<DataWriter>> data_writers;

    //! Actual statistical data reported by Fast DDS Statistics Module regarding this DomainParticipant.
    DomainParticipantData data;
};

/*
 * Topic entities hold data about the topics involved in the communication.
 */
struct Topic : Entity
{
    Topic(
            std::string topic_name,
            std::string topic_type,
            std::shared_ptr<Domain> topic_domain)
        : Entity(EntityKind::TOPIC, topic_name)
        , data_type(topic_type)
        , domain(topic_domain)
    {
    }

    //! The data type name of the topic
    std::string data_type;

    //! Reference to the Domain in which this topic is published/subscribed.
    std::shared_ptr<Domain> domain;

    /*
     * Collection of Datareaders subscribing to this topic.
     * The collection is ordered by the EntityId of the Datareader nodes.
     */
    std::map<EntityId, std::shared_ptr<DataReader>> data_readers;

    /*
     * Collection of DataWriters publishind in this topic.
     * The collection is ordered by the EntityId of the DataWriter nodes.
     */
    std::map<EntityId, std::shared_ptr<DataWriter>> data_writers;
};

/*
 * Base class for the DDS Endpoints.
 */
struct DDSEndpoint : DDSEntity
{
    DDSEndpoint(
            EntityKind entity_kind = EntityKind::INVALID,
            std::string endpoint_name = "INVALID",
            Qos endpoint_qos = {},
            std::string endpoint_guid = "|GUID UNKNOWN|",
            std::shared_ptr<DomainParticipant> endpoint_participant = nullptr,
            std::shared_ptr<Topic> endpoint_topic = nullptr)
        : DDSEntity(entity_kind, endpoint_name, endpoint_qos, endpoint_guid)
        , participant(endpoint_participant)
        , topic(endpoint_topic)
    {
    }

    //! Reference to the DomainParticipant in which this Endpoint runs.
    std::shared_ptr<DomainParticipant> participant;

    //! Reference to the Domain in which this endpoint publishes/subscribes.
    std::shared_ptr<Topic> topic;

    /*
     * Collection of Locators related to this endpoint.
     * The collection is ordered by the EntityId of the Locator nodes.
     */
    std::map<EntityId, std::shared_ptr<Locator>> locators;
};

/*
 * DataReader entities hold data about the DataReaders involved in the communication.
 */
struct DataReader : DDSEndpoint
{
    DataReader(
            std::string datareader_name,
            Qos datareader_qos,
            std::string datareader_guid,
            std::shared_ptr<DomainParticipant> datareader_participant,
            std::shared_ptr<Topic> datareader_topic
            )
        : DDSEndpoint(EntityKind::DATAREADER, datareader_name, datareader_qos, datareader_guid, datareader_participant,
                datareader_topic)
    {
    }

    //! Actual statistical data reported by Fast DDS Statistics Module regarding this DataReader.
    DataReaderData data;
};

/*
 * DataWriter entities hold data about the DataWriters involved in the communication.
 */
struct DataWriter : DDSEndpoint
{
    DataWriter(
            std::string datawriter_name,
            Qos datawriter_qos,
            std::string datawriter_guid,
            std::shared_ptr<DomainParticipant> datawriter_participant,
            std::shared_ptr<Topic> datawriter_topic
            )
        : DDSEndpoint(EntityKind::DATAWRITER, datawriter_name, datawriter_qos, datawriter_guid, datawriter_participant,
                datawriter_topic)
    {
    }

    //! Actual statistical data reported by Fast DDS Statistics Module regarding this DataWriter.
    DataWriterData data;
};

/*
 * Locator entities hold data about the Locators involved in the communication.
 *
 */
struct Locator : Entity
{
    Locator(
            std::string locator_name)
        : Entity(EntityKind::LOCATOR, locator_name)
    {
    }

    /*
     * Collection of DataReaders using this locator.
     * The collection is ordered by the EntityId of the DataReader nodes.
     */
    std::map<EntityId, std::shared_ptr<DataReader>> data_readers;

    /*
     * Collection of DataWriters using this locator.
     * The collection is ordered by the EntityId of the DataWriter nodes.
     */
    std::map<EntityId, std::shared_ptr<DataWriter>> data_writers;

    //! Actual statistical data reported by Fast DDS Statistics Module regarding this locator.
    LocatorData data;
};

} //namespace database
} //namespace statistics_backend
} //namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_ENTITIES_HPP_