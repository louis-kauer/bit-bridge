#ifndef BIT_BRIDGE_HEALTH_CHECKER_HPP
#define BIT_BRIDGE_HEALTH_CHECKER_HPP

#include "HealthCheckConfig.hpp"
#include "ServicePool.hpp"

#include <boost/asio.hpp>
#include <memory>
#include <thread>

class HealthChecker {
public:
    HealthChecker(boost::asio::io_context &ioContext,
                  ServicePool &pool,
                  const HealthCheckConfig &config);

    HealthChecker(const HealthChecker &) = delete;

    HealthChecker(HealthChecker &&) = delete;

    HealthChecker &operator=(const HealthChecker &) = delete;

    HealthChecker &operator=(HealthChecker &&) = delete;

    ~HealthChecker() = default;

    void Start();

    void Stop();

private:
    void ScheduleNextRound();

    void ProbeAll();

    void ProbeOne(size_t index);

    void OnProbeResult(size_t index,
                       const std::shared_ptr<boost::asio::ip::tcp::socket> &socket,
                       const boost::system::error_code &ec);

    boost::asio::io_context &m_ioContext;
    ServicePool &m_pool;
    HealthCheckConfig m_config;
    boost::asio::steady_timer m_timer;
    bool m_stopped{false};
};

#endif //BIT_BRIDGE_HEALTH_CHECKER_HPP