#include "MainFrame.hpp"
#include "YamlConfigSerializer.hpp"
#include "ServiceNode.hpp"

#include <wx/filename.h>
#include <wx/statline.h>
#include <array>
#include <vector>

MainFrame::MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size,
                     std::unique_ptr<AppSettings> settings)
    : wxFrame(nullptr, wxID_ANY, title, pos, size,
              wxDEFAULT_FRAME_STYLE & ~wxMAXIMIZE_BOX & ~wxRESIZE_BORDER)
      , m_settings(std::move(settings))
      , m_config(std::make_unique<LoadBalancerConfig>())
      , m_serializer(std::make_unique<YamlConfigSerializer>())
      , m_serviceList(nullptr)
      , m_nameInput(nullptr)
      , m_ipInput(nullptr)
      , m_portInput(nullptr)
      , m_weightInput(nullptr)
      , m_addButton(nullptr)
      , m_removeButton(nullptr)
      , m_saveButton(nullptr)
      , m_loadButton(nullptr)
      , m_emptyLabel(nullptr)
      , m_unsavedLabel(nullptr)
      , m_hasUnsavedChanges(false) {
    auto *mainVertical = new wxBoxSizer(wxVERTICAL); //NOLINT

    // Service list — multi-select enabled
    m_serviceList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                   wxLC_REPORT);
    m_serviceList->InsertColumn(0, "Name", wxLIST_FORMAT_LEFT, 140);
    m_serviceList->InsertColumn(1, "IP Address", wxLIST_FORMAT_LEFT, 160);
    m_serviceList->InsertColumn(2, "Port", wxLIST_FORMAT_LEFT, 70);
    m_serviceList->InsertColumn(3, "Weight", wxLIST_FORMAT_LEFT, 70);
    mainVertical->Add(m_serviceList, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 8);

    // Empty state label (overlaid on top of the list, positioned in RefreshServiceList)
    m_emptyLabel = new wxStaticText(m_serviceList, wxID_ANY, "No services added yet.",
                                    wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
    m_emptyLabel->SetForegroundColour(wxColour(128, 128, 128));

    // Separator
    mainVertical->Add(new wxStaticLine(this), 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 8);

    // Input row — fields expand proportionally to fill width
    auto *inputRow = new wxBoxSizer(wxHORIZONTAL); //NOLINT

    inputRow->Add(new wxStaticText(this, wxID_ANY, "Name:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    m_nameInput = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    inputRow->Add(m_nameInput, 3, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);

    inputRow->Add(new wxStaticText(this, wxID_ANY, "IP:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    m_ipInput = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    inputRow->Add(m_ipInput, 3, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);

    inputRow->Add(new wxStaticText(this, wxID_ANY, "Port:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    m_portInput = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    inputRow->Add(m_portInput, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);

    inputRow->Add(new wxStaticText(this, wxID_ANY, "Weight:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
    m_weightInput = new wxTextCtrl(this, wxID_ANY, "1", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    inputRow->Add(m_weightInput, 1, wxALIGN_CENTER_VERTICAL);

    mainVertical->Add(inputRow, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 8);

    // Button row — spread across the full width
    auto *buttonRow = new wxBoxSizer(wxHORIZONTAL); //NOLINT

    m_addButton = new wxButton(this, static_cast<int>(BitBridgeIds::ID_ADD_SERVICE), "Add Service");
    buttonRow->Add(m_addButton, 1, wxRIGHT, 4);

    m_removeButton = new wxButton(this, static_cast<int>(BitBridgeIds::ID_REMOVE_SERVICE), "Remove Selected");
    buttonRow->Add(m_removeButton, 1, wxRIGHT, 4);

    m_saveButton = new wxButton(this, static_cast<int>(BitBridgeIds::ID_SAVE_CONFIG), "Save Configuration");
    buttonRow->Add(m_saveButton, 1, wxRIGHT, 4);

    m_loadButton = new wxButton(this, static_cast<int>(BitBridgeIds::ID_LOAD_CONFIG), "Load Configuration");
    buttonRow->Add(m_loadButton, 1);

    mainVertical->Add(buttonRow, 0, wxEXPAND | wxALL, 8);

    // Status bar
    wxFrameBase::CreateStatusBar(2);
    wxFrameBase::SetStatusText("Ready", 0);
    const std::array<int, 2> fieldWidths = {-1, 150};
    wxFrameBase::GetStatusBar()->SetStatusWidths(2, fieldWidths.data());

    // Orange unsaved indicator overlaid on the second status bar field
    m_unsavedLabel = new wxStaticText(wxFrameBase::GetStatusBar(), wxID_ANY, "Unsaved Changes");
    m_unsavedLabel->SetForegroundColour(wxColour(230, 160, 0));
    m_unsavedLabel->SetFont(m_unsavedLabel->GetFont().Bold());
    m_unsavedLabel->Hide();

    SetSizer(mainVertical);

    // Set tab order: Name → IP → Port → Weight → Add → Remove → Save → Load → List
    m_nameInput->MoveAfterInTabOrder(m_serviceList);
    m_ipInput->MoveAfterInTabOrder(m_nameInput);
    m_portInput->MoveAfterInTabOrder(m_ipInput);
    m_weightInput->MoveAfterInTabOrder(m_portInput);
    m_addButton->MoveAfterInTabOrder(m_weightInput);
    m_removeButton->MoveAfterInTabOrder(m_addButton);
    m_saveButton->MoveAfterInTabOrder(m_removeButton);
    m_loadButton->MoveAfterInTabOrder(m_saveButton);

    // Bind button events
    m_addButton->Bind(wxEVT_BUTTON, &MainFrame::OnAddService, this);
    m_removeButton->Bind(wxEVT_BUTTON, &MainFrame::OnRemoveService, this);
    m_saveButton->Bind(wxEVT_BUTTON, &MainFrame::OnSaveConfig, this);
    m_loadButton->Bind(wxEVT_BUTTON, &MainFrame::OnLoadConfig, this);

    // Bind Enter key on input fields to trigger Add Service
    m_nameInput->Bind(wxEVT_TEXT_ENTER, &MainFrame::OnAddService, this);
    m_ipInput->Bind(wxEVT_TEXT_ENTER, &MainFrame::OnAddService, this);
    m_portInput->Bind(wxEVT_TEXT_ENTER, &MainFrame::OnAddService, this);
    m_weightInput->Bind(wxEVT_TEXT_ENTER, &MainFrame::OnAddService, this);

    // Frame-level Tab navigation
    Bind(wxEVT_CHAR_HOOK, &MainFrame::HandleTabNavigation, this);

    // Reposition empty label when list resizes
    m_serviceList->Bind(wxEVT_SIZE, [this]([[maybe_unused]] wxSizeEvent &event) {
        event.Skip();
        RepositionEmptyLabel();
    });

    // Bind Delete key on list to trigger remove
    m_serviceList->Bind(wxEVT_KEY_DOWN, [this](wxKeyEvent &event) {
        if (event.GetKeyCode() == WXK_DELETE || event.GetKeyCode() == WXK_BACK) {
            wxCommandEvent dummy;
            OnRemoveService(dummy);
        } else {
            event.Skip();
        }
    });

    // Apply defaults from settings, then load existing config if present
    ApplyDefaultConfig();
    LoadExistingConfig();
    m_nameInput->SetFocus();
}

void MainFrame::HandleTabNavigation(wxKeyEvent &event) {
    if (event.GetKeyCode() != WXK_TAB) {
        event.Skip();
        return;
    }

    const wxWindow *focused = FindFocus();
    const std::array<wxWindow *, 4> tabOrder = {
        m_nameInput, m_ipInput, m_portInput, m_weightInput
    };

    for (size_t i = 0; i < tabOrder.size(); ++i) {
        if (focused == tabOrder[i]) {
            size_t next = event.ShiftDown()
                              ? (i + tabOrder.size() - 1) % tabOrder.size()
                              : (i + 1) % tabOrder.size();
            tabOrder[next]->SetFocus();
            return;
        }
    }

    event.Skip();
}

void MainFrame::RepositionEmptyLabel() const {
    if (!m_emptyLabel->IsShown()) {
        return;
    }
    const wxSize listSize = m_serviceList->GetClientSize();
    const wxSize labelSize = m_emptyLabel->GetBestSize();
    const int x = (listSize.GetWidth() - labelSize.GetWidth()) / 2;
    const int y = (listSize.GetHeight() - labelSize.GetHeight()) / 3;
    m_emptyLabel->SetPosition(wxPoint(x, y));
}

void MainFrame::ApplyDefaultConfig() const {
    m_config->SetName(m_settings->GetDefaultName());
    m_config->SetListenAddress(m_settings->GetDefaultListenAddress());
    m_config->SetListenPort(m_settings->GetDefaultListenPort());
    m_config->SetRoutingAlgorithm(m_settings->GetDefaultRoutingAlgorithm());

    m_config->SetHealthCheck(HealthCheckConfig(
        m_settings->GetDefaultHealthCheckEnabled(),
        m_settings->GetDefaultHealthCheckIntervalMs(),
        m_settings->GetDefaultHealthCheckTimeoutMs(),
        m_settings->GetDefaultHealthCheckUnhealthyThreshold()));

    m_config->SetConnection(ConnectionConfig(
        m_settings->GetDefaultMaxPerService(),
        m_settings->GetDefaultIdleTimeoutMs(),
        m_settings->GetDefaultConnectTimeoutMs()));
}

void MainFrame::LoadExistingConfig() {
    const std::string configPath = m_settings->GetConfigFilePath();
    if (!wxFileName::Exists(configPath)) {
        return;
    }

    if (m_serializer->Load(*m_config, configPath)) {
        RefreshServiceList();
        MarkSaved();
        SetStatusText(wxString::Format("Configuration loaded from %s", configPath.c_str()));
    }
}

void MainFrame::MarkUnsaved() {
    m_hasUnsavedChanges = true;
    wxRect rect;
    GetStatusBar()->GetFieldRect(1, rect);
    const wxSize labelSize = m_unsavedLabel->GetBestSize();
    const int x = rect.x + rect.width - labelSize.GetWidth() - 4;
    const int y = rect.y + (rect.height - labelSize.GetHeight()) / 2;
    m_unsavedLabel->SetPosition(wxPoint(x, y));
    m_unsavedLabel->Show();
    SetStatusText("", 1);
}

void MainFrame::MarkSaved() {
    m_hasUnsavedChanges = false;
    m_unsavedLabel->Hide();
    SetStatusText("", 1);
}

void MainFrame::OnAddService([[maybe_unused]] wxCommandEvent &event) {
    wxString name = m_nameInput->GetValue();
    wxString ip = m_ipInput->GetValue();
    wxString portStr = m_portInput->GetValue();
    wxString weightStr = m_weightInput->GetValue();

    if (!ValidateInput(name, ip, portStr, weightStr)) {
        return;
    }

    unsigned long portVal = 0;
    portStr.ToULong(&portVal);
    unsigned long weightVal = 1;
    weightStr.ToULong(&weightVal);

    // Check for duplicate name or IP:port
    const auto &services = m_config->GetServices();
    for (const auto &existing: services) {
        if (existing.GetName() == name.ToStdString()) {
            wxMessageBox(wxString::Format("A service named '%s' already exists.", name),
                         "Duplicate Service", wxOK | wxICON_WARNING);
            return;
        }
        if (existing.GetIp() == ip.ToStdString() && existing.GetPort() == static_cast<uint16_t>(portVal)) {
            wxMessageBox(wxString::Format("A service with address %s:%lu already exists.", ip, portVal),
                         "Duplicate Service", wxOK | wxICON_WARNING);
            return;
        }
    }

    ServiceNode service(name.ToStdString(), ip.ToStdString(),
                        static_cast<uint16_t>(portVal), static_cast<uint16_t>(weightVal));

    m_config->AddService(service);
    RefreshServiceList();
    MarkUnsaved();

    m_nameInput->Clear();
    m_ipInput->Clear();
    m_portInput->Clear();
    m_weightInput->SetValue("1");
    m_nameInput->SetFocus();

    SetStatusText(wxString::Format("Service added: %s (%s:%s)", name, ip, portStr));
}

void MainFrame::OnRemoveService([[maybe_unused]] wxCommandEvent &event) {
    // Collect all selected indices
    std::vector<long> selected;
    long item = -1;
    while ((item = m_serviceList->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) != -1) {
        selected.push_back(item);
    }

    if (selected.empty()) {
        wxMessageBox("Please select one or more services to remove.", "No Selection",
                     wxOK | wxICON_INFORMATION);
        return;
    }

    // Remove in reverse order so indices stay valid
    for (auto it = selected.rbegin(); it != selected.rend(); ++it) {
        m_config->RemoveService(static_cast<size_t>(*it));
    }

    RefreshServiceList();
    MarkUnsaved();

    const wxString msg = selected.size() == 1
                             ? "1 service removed"
                             : wxString::Format("%zu services removed", selected.size());
    SetStatusText(msg);
}

void MainFrame::OnLoadConfig([[maybe_unused]] wxCommandEvent &event) {
    const std::string configPath = m_settings->GetConfigFilePath();
    if (!wxFileName::Exists(configPath)) {
        wxMessageBox(wxString::Format("Configuration file not found: %s", configPath.c_str()),
                     "Load Error", wxOK | wxICON_WARNING);
        return;
    }

    if (auto newConfig = std::make_unique<LoadBalancerConfig>(); m_serializer->Load(*newConfig, configPath)) {
        m_config = std::move(newConfig);
        RefreshServiceList();
        MarkSaved();
        SetStatusText(wxString::Format("Configuration loaded from %s", configPath.c_str()));
    } else {
        wxMessageBox("Failed to parse configuration file.", "Load Error",
                     wxOK | wxICON_ERROR);
    }
}

void MainFrame::OnSaveConfig([[maybe_unused]] wxCommandEvent &event) {
    if (const std::string configPath = m_settings->GetConfigFilePath(); m_serializer->Save(*m_config, configPath)) {
        MarkSaved();
        SetStatusText(wxString::Format("Configuration saved to %s", configPath.c_str()));
    } else {
        wxMessageBox("Failed to save configuration file.", "Save Error",
                     wxOK | wxICON_ERROR);
    }
}

void MainFrame::RefreshServiceList() const {
    m_serviceList->DeleteAllItems();
    const auto &services = m_config->GetServices();
    for (size_t i = 0; i < services.size(); ++i) {
        const long index = m_serviceList->InsertItem(static_cast<long>(i), services[i].GetName());
        m_serviceList->SetItem(index, 1, services[i].GetIp());
        m_serviceList->SetItem(index, 2, std::to_string(services[i].GetPort()));
        m_serviceList->SetItem(index, 3, std::to_string(services[i].GetWeight()));
    }

    const bool empty = services.empty();
    m_emptyLabel->Show(empty);
    if (empty) {
        RepositionEmptyLabel();
    }
}

bool MainFrame::ValidateInput(const wxString &name, const wxString &ip,
                              const wxString &port, const wxString &weight) {
    if (name.IsEmpty()) {
        wxMessageBox("Service name cannot be empty.", "Validation Error",
                     wxOK | wxICON_WARNING);
        return false;
    }

    if (ip.IsEmpty()) {
        wxMessageBox("IP address cannot be empty.", "Validation Error",
                     wxOK | wxICON_WARNING);
        return false;
    }

    if (port.IsEmpty()) {
        wxMessageBox("Port cannot be empty.", "Validation Error",
                     wxOK | wxICON_WARNING);
        return false;
    }

    unsigned long portVal = 0;
    if (!port.ToULong(&portVal) || portVal == 0 || portVal > 65535) {
        wxMessageBox("Port must be a number between 1 and 65535.", "Validation Error",
                     wxOK | wxICON_WARNING);
        return false;
    }

    if (unsigned long weightVal = 0; !weight.ToULong(&weightVal) || weightVal == 0 || weightVal > 65535) {
        wxMessageBox("Weight must be a number between 1 and 65535.", "Validation Error",
                     wxOK | wxICON_WARNING);
        return false;
    }

    // Basic IPv4 validation
    if (ServiceNode const testNode("test", ip.ToStdString(), static_cast<uint16_t>(portVal), 1); !testNode.IsValid()) {
        wxMessageBox("Invalid IP address format. Expected: x.x.x.x (0-255 per octet).",
                     "Validation Error", wxOK | wxICON_WARNING);
        return false;
    }

    return true;
}